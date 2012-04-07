// annotcloud/annotpaint.cc
// 4/6/2012

#include "module/annotcloud/annotpaint.h"
#include "module/qtext/datetime.h"
#include <QPainter>
#include <QHash>
#include <QDateTime>
#ifdef WITH_QT_SVG
#  include <QSvgGenerator>
#endif // WITH_QT_SVG
#include <climits>

#define DEBUG "annotpaint"
#include "module/debug/debug.h"

void
AnnotCloud::
AnnotationPainter::paintTitle(QPainter &p, const QString &text, const QColor &color, int fontSize, const QRect &canvas) const
{
  enum { FontSize = 20 }; // 20em
  if (!fontSize)
    fontSize = FontSize;

  int width = canvas.width() ? canvas.width() : p.device()->width(),
      height = canvas.height() ? canvas.height() : p.device()->height();

 if (color.isValid())
    p.setPen(color);
 else
    p.setPen(Qt::cyan);

  QTextOption opt;
  opt.setAlignment(Qt::AlignCenter);
  QFont f = p.font();
  f.setItalic(true);
  f.setBold(true);
  f.setPointSize(fontSize);
  p.setFont(f);
  p.drawText(QRectF(canvas.x(), canvas.y(), width, height), text, opt);
}

void
AnnotCloud::
AnnotationPainter::paintHistogram(QPainter &p, const AnnotationList &l, Annotation::Field sortBy, const QString &title, const QRect &canvas) const
{
  switch (sortBy) {
  case Annotation::Pos: paintHistogramByPos(p, l, title, canvas); break;
  case Annotation::CreateTime: paintHistogramByCreateTime(p, l, title, canvas); break;
  default: Q_ASSERT(0); break;
  }
}

void
AnnotCloud::
AnnotationPainter::paintHistogramByPos(QPainter &p, const AnnotationList &l, const QString &title, const QRect &canvas) const
{
  enum { metric = 10 * 1000 }; // every 10 seconds
  enum { alpha = int(255 * 0.9),
         r_start = 255, r_stop = 255,
         g_start = 255, g_stop = 0,
         b_start = 0, b_stop = 96 };

  enum { MarginSize = 3, LabelHeight = 12 };
  enum { LabelMetric = 5 * 6 }; // every 5 min

  enum { FontSize = 10 }; // 10em

  if (l.isEmpty())
    return;

  typedef QHash<int, int> Histogram;
  Histogram hist;
  int maxX = 0, maxY = 0;
  foreach (const Annotation &a, l) {
    int x = a.pos() / metric;
    if (x < 0)
      continue;
    int y = ++hist[x];
    if (maxX < x)
      maxX = x;
    if (maxY < y)
      maxY = y;
  }

  // Draw histogram
  int width = canvas.width() ? canvas.width() : p.device()->width(),
      height = canvas.height() ? canvas.height() : p.device()->height();
  int histHeight = height - (MarginSize + LabelHeight);
  for (Histogram::ConstIterator i = hist.begin(); i != hist.end(); ++i) {
    int x = i.key(),
        y = i.value();

    qreal px = x / (qreal)maxX,
          py = y / (qreal)maxY;
    x = width * px;
    y = histHeight * py;

    int r = r_start + (r_stop - r_start) * py,
        g = g_start + (g_stop - g_start) * py,
        b = b_start + (b_stop - b_start) * py;
    QColor color(r, g, b, alpha);

    p.setPen(color);
    p.drawLine(canvas.x() + x, canvas.y() + histHeight, canvas.x() + x, canvas.y() + histHeight - y);
  }

  // Draw label
  p.setPen(Qt::cyan);
  QFont f = p.font();
  f.setPointSize(FontSize);
  p.setFont(f);
  int labelCount = maxX / LabelMetric;
  int labelWidth = width / labelCount;
  for (int i = 0; i < labelCount; i++) {
    qint64 msecs = i * LabelMetric * metric;
    QString label = QtExt::msecs2time(msecs).toString();
    int x = canvas.x() + MarginSize + i * labelWidth,
        y = canvas.y() + height - LabelHeight;
    p.drawText(QRectF(x, y, labelWidth, LabelHeight), label);
  }

  // Draw title
  if (!title.isEmpty()) {
    enum { TitleHeight = 20, NoteHeight = 10, NoteMargin = 5 };
    enum { TitleFont = 20, NoteFont = 10 };
    QColor titleColor = Qt::cyan,
           noteColor = QColor("orange");

    QString peak = QString::number(maxY),
            average = QString().sprintf("%.2f", l.size() / (qreal)maxX);

    int titleHeight = qMin(height, (int)TitleHeight);
    paintTitle(p, title, titleColor, TitleFont, QRect(0, 0, width, titleHeight));

    if (height > TitleHeight + NoteHeight + NoteMargin) {
      QString note = QString("average: %1 / 10secs.  peak: %2 / 10secs.")
                     .arg(average).arg(peak);
      paintTitle(p, note, noteColor, NoteFont, QRect(0, TitleHeight + NoteMargin, width, NoteHeight));
    }
  }
}

void
AnnotCloud::
AnnotationPainter::paintHistogramByCreateTime(QPainter &p, const AnnotationList &l, const QString &title, const QRect &canvas) const
{
  enum { metric = 6 * 3600 }; // every 6 hours
  enum { alpha = int(255 * 0.9),
         r_start = 255, r_stop = 255,
         g_start = 255, g_stop = 0,
         b_start = 0, b_stop = 96 };

  enum { MarginSize = 10, LabelHeight = 30 };
  enum { LabelMetric = 30 * 4 }; // every month

  enum { FontSize = 10 }; // 5em

  if (l.isEmpty())
    return;

  typedef QHash<qint64, int> Histogram;
  Histogram hist;
  qint64 maxX = 0,
         minX = LLONG_MAX;
  int maxY = 0;
  foreach (const Annotation &a, l) {
    qint64 x = a.createTime() / metric;
    if (x <= 0)
      continue;
    int y = ++hist[x];
    if (minX > x)
      minX = x;
    if (maxX < x)
      maxX = x;
    if (maxY < y)
      maxY = y;
  }
  if (maxX == minX)
    minX = maxX - LabelMetric;

  // Draw histogram
  int width = canvas.width() ? canvas.width() : p.device()->width(),
      height = canvas.height() ? canvas.height() : p.device()->height();
  int histHeight = height - (MarginSize + LabelHeight);
  for (Histogram::ConstIterator i = hist.begin(); i != hist.end(); ++i) {
    qint64 x = i.key();
    int y = i.value();

    qreal px = (x - minX) / (qreal)(maxX - minX),
          py = y / (qreal)maxY;
    x = width * px;
    y = histHeight * py;

    int r = r_start + (r_stop - r_start) * py,
        g = g_start + (g_stop - g_start) * py,
        b = b_start + (b_stop - b_start) * py;
    QColor color(r, g, b, alpha);

    p.setPen(color);
    p.drawLine(canvas.x() + x, canvas.y() + histHeight, canvas.x() + x, canvas.y() + histHeight - y);
  }

  // Draw label
  p.setPen(Qt::cyan);
  QFont f = p.font();
  f.setPointSize(FontSize);
  p.setFont(f);
  int labelCount = (maxX - minX) / LabelMetric;
  int labelWidth = width / labelCount;
  for (int i = 0; i < labelCount; i++) {
    qint64 secs = (minX + i * LabelMetric) * metric;
    QString label = QDateTime::fromMSecsSinceEpoch(secs * 1000).toString("M/d/yyyy");
    int x = canvas.x() + MarginSize + i * labelWidth,
        y = canvas.y() + height - LabelHeight;
    p.save();
    p.translate(x, y);
    p.rotate(30);
    p.drawText(0, 0, label);
    p.restore();
  }

  // Draw title
  if (!title.isEmpty()) {
    enum { TitleHeight = 20, NoteHeight = 10, NoteMargin = 5 };
    enum { TitleFont = 20, NoteFont = 10 };
    QColor titleColor = Qt::cyan,
           noteColor = QColor("orange");

    QString peak = QString::number(maxY),
            average = QString().sprintf("%.2f", l.size() / (qreal)(maxX - minX));

    int titleHeight = qMin(height, (int)TitleHeight);
    paintTitle(p, title, titleColor, TitleFont, QRect(0, 0, width, titleHeight));

    if (height > TitleHeight + NoteHeight + NoteMargin) {
      QString note = QString("average: %1 / 6hrs.  peak: %2 / 6hrs.")
                     .arg(average).arg(peak);
      paintTitle(p, note, noteColor, NoteFont, QRect(0, TitleHeight + NoteMargin, width, NoteHeight));
    }
  }
}

// See: http://harmattan-dev.nokia.com/docs/library/html/qt4/painting-svggenerator.html
bool
AnnotCloud::
AnnotationPainter::saveHistogramAsFile(
    const QString &fileName,
    const AnnotationList &l, Annotation::Field sortBy,
   int width, int height,
    const QString &title, const QString &description) const
{
  enum { CanvasWidth = 400, CanvasHeight = 200 };
#ifdef WITH_QT_SVG
  DOUT("enter: fileName =" << fileName);

  if (width <= 0)
    width = CanvasWidth;
  if (height <= 0)
    height = CanvasHeight;

  QSvgGenerator generator;
  generator.setFileName(fileName);
  generator.setSize(QSize(width, height));
  generator.setViewBox(QRect(0, 0, width, height));
  if (!title.isEmpty())
    generator.setTitle(title);
  if (!description.isEmpty())
    generator.setDescription(description);

  QPainter painter;
  painter.begin(&generator);

  painter.fillRect(QRect(0, 0, width, height), Qt::transparent);

  paintHistogram(painter, l, sortBy, title);
  painter.end();

  DOUT("exit");
  return true;
#else
  Q_UNUSED(fileName)
  Q_UNUSED(width) Q_UNUSED(height)
  Q_UNUSED(l) Q_UNUSED(sortBy)
  Q_UNUSED(title) Q_UNUSED(description)
  return false;
#endif // WITH_QT_SVG
}

// EOF