// application.cc
// 11/18/2011
#include "application.h"
#include "global.h"
#ifdef Q_OS_WIN
#  include "win/qtwin/qtwin.h"
#endif // Q_OS_WIN
#ifdef Q_OS_MAC
#  include "mac/qtmac/qtmac.h"
#elif defined Q_OS_UNIX
extern "C" {
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <unistd.h>
#  include <fcntl.h>
} // extern "C"
#endif // Q_OS_
#include <QtGui>

#define DEBUG "application"
#include "module/debug/debug.h"

// - Constructions -

Application::Application(int &argc, char **argv, bool gui)
  : Base(argc, argv, gui), w_(0)
{
  DOUT("enter");
  setOrganizationDomain(G_DOMAIN);
  setOrganizationName(G_ORGANIZATION);
  setApplicationName(G_APPLICATION);
  setApplicationVersion(G_VERSION);

  createDirectories();
  DOUT("exit");
}

Application::~Application()
{
  DOUT("enter: abort in 3 seconds");
  QTimer::singleShot(3000, this, SLOT(abort()));
  DOUT("exit");
}

void
Application::createDirectories()
{
  QDir profile(G_PATH_PROFILE);
  if (!profile.exists())
    profile.mkpath(profile.absolutePath());
}

void
Application::abort()
{
  DOUT("enter");
  qint64 pid = applicationPid();
#ifdef Q_OS_WIN
  QtWin::killCurrentProcess();
  // If failed
  QProcess::startDetached(QString("tskill %1").arg(QString::number(pid)));
#else
  QProcess::startDetached(QString("kill -9 %1").arg(QString::number(pid)));
#endif // Q_OS_WIN
  DOUT("exit");
}

void
Application::abortAll()
{
  DOUT("enter");
  QString app = QFileInfo(applicationFilePath()).fileName();
#ifdef Q_OS_WIN
  QProcess::startDetached("tskill", QStringList(app));
#else
  QProcess::startDetached("killall", QStringList(app));
#endif // Q_OS_WIN
  DOUT("exit");
}

// - Properties -

bool
Application::isSingleInstance() const
{
#ifdef Q_WS_WIN
  QFileInfo fi(applicationFilePath());
  QString processName = fi.fileName();
  QList<ulong> pids = QtWin::getProcessIdsByName(processName);
  return pids.size() <= 1;
#elif defined Q_WS_MAC
  return true;
#else
  // See: http://www.linuxquestions.org/questions/programming-9/restricting-multiple-instance-of-a-program-242069/
  static int fd_lock = -1;
  if (fd_lock < 0) {
    fd_lock = ::open((G_PATH_LOCK_RUNNING).toLocal8Bit(), O_WRONLY|O_CREAT, 0666);
    if (fd_lock >= 0) {
      flock fl; {
        fl.l_type = F_WRLCK;
        fl.l_whence = SEEK_SET;
        fl.l_start = 0;
        fl.l_len = 1;
      }
      if (::fcntl(fd_lock, F_SETLK, &fl) < 0) {
        ::close(fd_lock);
        fd_lock = -1;
      }
    }
  }
  return fd_lock >= 0;
#endif // Q_OS_
}

// - Events -

bool
Application::event(QEvent *e)
{
  bool accept = true;
  switch (e->type()) {
  case QEvent::FileOpen: if (w_) sendEvent(w_, e); break;
  default: accept = Base::event(e);
  }

  return accept;
}

// EOF
