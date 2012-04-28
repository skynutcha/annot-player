// acaboutdialog.cc
// 11/13/2011

#include "project/common/acaboutdialog.h"
#include "project/common/acglobal.h"
#include "project/common/acui.h"
#include "module/qtext/htmltag.h"
#include <QtGui>

#define WINDOW_FLAGS_BASE \
  Qt::Dialog | \
  Qt::CustomizeWindowHint | \
  Qt::WindowStaysOnTopHint

#ifdef Q_WS_MAC
#  define WINDOW_FLAGS ( \
    Qt::FramelessWindowHint | \
    WINDOW_FLAGS_BASE )
#else
#  define WINDOW_FLAGS ( \
    Qt::WindowTitleHint | \
    WINDOW_FLAGS_BASE )
#endif // Q_WS_MAC

// - Constructions -

#ifdef Q_WS_MAC
#  define WINDOW_SIZE   QSize(300, 200)
#else
#  define WINDOW_SIZE   QSize(270, 160)
#endif // Q_WS_MAC

QString
AcAboutDialog::templateText() const
{
  static const char *ret =
    HTML_CENTER_OPEN() \
      HTML_STYLE_OPEN(color:purple) \
        HTML_BR() \
\
        AC_ORGANIZATION " " \
        "%1" " " \
        "%2" \
        HTML_BR() \
\
        HTML_EM_OPEN() \
          AC_LICENSE \
        HTML_EM_CLOSE() \
        HTML_BR() \
\
        HTML_BR() \
\
        HTML_A_OPEN("dummy") \
          "http://annot.me" HTML_BR() \
          AC_UPDATEPAGE     HTML_BR() \
          AC_EMAIL          HTML_BR() \
        HTML_A_CLOSE() \
\
      HTML_STYLE_CLOSE() \
    HTML_CENTER_CLOSE()
  ;
  return ret;
}

void
AcAboutDialog::init()
{
  setWindowTitle(tr("About"));
  createLayout();
}

void
AcAboutDialog::createLayout()
{
  AcUi::globalInstance()->setWindowStyle(this);

  // Components

  QToolButton *okButton = AcUi::globalInstance()->makeToolButton(
       AcUi::PushHint, tr("OK"), this, SLOT(hide()));

  textEdit_ = AcUi::globalInstance()->makeTextEdit(
      AcUi::ReadOnlyHint, tr("About"));

  // Layout

  QVBoxLayout *col = new QVBoxLayout; {
    col->addWidget(textEdit_);
    col->addWidget(okButton, 0, Qt::AlignHCenter); // stretch = 0
  } setLayout(col);

  // l, t, r, b
  int patch = 0;
  if (!AcUi::isAeroAvailable())
    patch = 4;
  col->setContentsMargins(patch, patch, patch, 0);
  setContentsMargins(4, 4, 4, patch);

  resize(WINDOW_SIZE);

  // Shortcuts
  connect(new QShortcut(QKeySequence("Esc"), this), SIGNAL(activated()), SLOT(hide()));
  connect(new QShortcut(QKeySequence("CTRL+W"), this), SIGNAL(activated()), SLOT(hide()));

  // Focus
  okButton->setFocus();
}

void
AcAboutDialog::setVisible(bool visible)
{
  if (visible)
    textEdit_->setHtml(text());
  Base::setVisible(visible);
}

// EOF
