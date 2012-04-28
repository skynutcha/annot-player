// qtext/webpage.cc
// 4/9/2012

#include "module/qtext/webpage.h"
#include <QtWebKit/QWebFrame>
#include <QtGui>

//#define HOMEPAGE_URL    "http://annot.me"
//#define HOMEPAGE_URL    "http://" ANNOT_HOST_IP
//#define BASE_URL        HOMEPAGE_URL "/api/player/"
//#define BASE_URL        ""

#define DEBUG "qtext::webpage"
#include "module/debug/debug.h"

#ifdef __GNUC__
#  pragma GCC diagnostic ignored "-Wparentheses" // suggest parentheses around assignment
#endif // __GNUC__

// - RC -

#ifdef Q_OS_LINUX
#  ifndef DOCDIR
#    define DOCDIR      "/usr/share/annot"
#  endif // DOCDIR
#  define RC_PREFIX     DOCDIR "/"
#else
#  define RC_PREFIX     QCoreApplication::applicationDirPath() + "/doc/"
#endif // Q_OS_LINUX

#define RC_HTML_ERROR   RC_PREFIX "error.html"

namespace { // anonymous
  inline QByteArray rc_html_error_()
  {
    QFile f(RC_HTML_ERROR);
    return f.open(QIODevice::ReadOnly) ? f.readAll() : QByteArray();
  }
} // anonymous namespace

// - Construction -

QtExt::
WebPage::WebPage(QWidget *parent)
  : Base(parent)
{ connect(this, SIGNAL(linkHovered(QString,QString,QString)), SLOT(setHoveredLink(QString))); }

// - Events -

bool
QtExt::
WebPage::event(QEvent *event)
{
  Q_ASSERT(event);
  if (event->type() == QEvent::MouseButtonRelease) {
    QMouseEvent *e = static_cast<QMouseEvent *>(event);
    if (e->button() == Qt::LeftButton && e->modifiers() == Qt::ControlModifier)  {
      if (!hoveredLink_.isEmpty())
        emit openLinkRequested(hoveredLink_);
      e->accept();
      return true;
    }
  }
  return Base::event(event);
}


// - Extensions -

bool
QtExt::
WebPage::supportsExtension(Extension extension) const
{
  switch (extension) {
  case ErrorPageExtension: return true;
  default: return Base::supportsExtension(extension);
  }
}

bool
QtExt::
WebPage::extension(Extension extension, const ExtensionOption *option, ExtensionReturn *output)
{
  switch (extension) {
  case ErrorPageExtension:
    return errorPageExtension(static_cast<const ErrorPageExtensionOption *>(option),
                              static_cast<ErrorPageExtensionReturn *>(output))
        || Base::extension(extension, option, output);
  default:
    return Base::extension(extension, option, output);
  }
}

bool
QtExt::
WebPage::errorPageExtension(const ErrorPageExtensionOption *option, ErrorPageExtensionReturn *output)
{
  if (!option || !output)
    return false;
  DOUT("enter: error =" << option->error << ", message =" << option->errorString);
  output->baseUrl = option->url;
  output->content = rc_html_error_();
  output->contentType = "text/html";
  output->encoding = "UTF-8";
  DOUT("exit");
  return true;
}

// - Scroll -

void
QtExt::
WebPage::scrollTop()
{ mainFrame()->setScrollBarValue(Qt::Vertical, 0); }

void
QtExt::
WebPage::scrollBottom()
{ mainFrame()->setScrollBarValue(Qt::Vertical, mainFrame()->scrollBarMaximum(Qt::Vertical)); }

void
QtExt::
WebPage::scrollLeft()
{ mainFrame()->setScrollBarValue(Qt::Horizontal, 0); }

void
QtExt::
WebPage::scrollRight()
{ mainFrame()->setScrollBarValue(Qt::Horizontal, mainFrame()->scrollBarMaximum(Qt::Horizontal)); }

// EOF

/*
QObject*
QtExt::
WebPage::createPlugin(const QString &classid, const QUrl &url, const QStringList &paramNames, const QStringList &paramValues)
{
  DOUT("classId =" << classid << ", url =" << url.toString() << ", paramNames =" << paramNames << ", paramValues =" << paramValues);
  return Base::createPlugin(classid, url, paramNames, paramValues);
}
*/
