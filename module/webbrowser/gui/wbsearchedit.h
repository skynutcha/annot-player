#ifndef WBSEARCHEDIT_H
#define WBSEARCHEDIT_H

// gui/wbsearchedit.h
// 3/31/2012

#include "wbcomboedit.h"

class WbSearchEngine;

class WbSearchEdit : public WbComboEdit
{
  Q_OBJECT
  typedef WbSearchEdit Self;
  typedef WbComboEdit Base;

public:
  explicit WbSearchEdit(QWidget *parent = 0)
    : Base(parent), engine_(0) { init(); }
  explicit WbSearchEdit(const QStringList &items, QWidget *parent = 0)
    : Base(items, parent), engine_(0) { init(); }

  void setEngines(const QList<WbSearchEngine *> engines)
  { engines_ = engines; invalidateEngines(); }

  QStringList recent() const;

signals:
  void engineChanged(int engine);

  // - Actions -
public slots:
  void setText(const QString &text);
  void clearText() { setText(QString()); }
  void setEngine(int engine);
  void addRecent(const QString &text);
  void removeRecent(const QString &text);

  int recentCount() const { return count() - engines_.size(); }

protected slots:
  void invalidateEngines();
  void setEngineByIndex(int index);
  virtual void submitText(); ///< \override

  void updateText(const QString &text);

private:
  void init()
  { createActions(); createConnections(); }

  void createActions();
  void createConnections();
protected:
  int engine_;

  QList<WbSearchEngine *> engines_;

  QString currentText_, lastText_;
};

#endif // WBSEARCHEDIT_H
