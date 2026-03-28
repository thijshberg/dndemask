#include "dndemask.hpp"
#include "maskfile.hpp"
#include <QApplication>
#include <QBuffer>
#include <QFileDialog>
#include <QMainWindow>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QScreen>
#include <QToolBar>
#include <QWidget>

namespace dndemask {

class ImageCanvas : public QWidget {
  QPixmap m_original;
  QPixmap m_display;
  QPoint m_dragStart;
  QRect m_selection;
  bool m_dragging = false;
  QVector<QRect> m_boxes; // in original image coordinates
  QString m_sourceFilename;

  QRect displayRect() const {
    return QRect((width() - m_display.width()) / 2,
                 (height() - m_display.height()) / 2,
                 m_display.width(), m_display.height());
  }

  QRect toImageRect(QRect widgetRect) const {
    QRect dr = displayRect();
    double sx = (double)m_original.width() / m_display.width();
    double sy = (double)m_original.height() / m_display.height();
    return QRect(QPoint((widgetRect.left() - dr.left()) * sx,
                        (widgetRect.top() - dr.top()) * sy),
                 QPoint((widgetRect.right() - dr.left()) * sx,
                        (widgetRect.bottom() - dr.top()) * sy))
        .intersected(m_original.rect());
  }

  void updateDisplay() {
    m_display = m_original.scaled(size(), Qt::KeepAspectRatio,
                                  Qt::SmoothTransformation);
    update();
  }

protected:
  void resizeEvent(QResizeEvent *) override { updateDisplay(); }

  QRect toDisplayBox(const QRect &imageBox) const {
    QRect dr = displayRect();
    double sx = (double)m_display.width() / m_original.width();
    double sy = (double)m_display.height() / m_original.height();
    return QRect(dr.left() + imageBox.x() * sx, dr.top() + imageBox.y() * sy,
                 imageBox.width() * sx, imageBox.height() * sy);
  }

  void paintEvent(QPaintEvent *) override {
    QPainter p(this);
    p.drawPixmap(displayRect().topLeft(), m_display);
    for (const QRect &box : m_boxes)
      p.fillRect(toDisplayBox(box), Qt::black);
    if (m_dragging && m_selection.isValid()) {
      p.setPen(QPen(Qt::white, 1, Qt::DashLine));
      p.drawRect(m_selection);
    }
  }

  void mousePressEvent(QMouseEvent *e) override {
    m_dragStart = e->pos();
    m_selection = {};
    m_dragging = true;
  }

  void mouseMoveEvent(QMouseEvent *e) override {
    if (m_dragging) {
      m_selection = QRect(m_dragStart, e->pos()).normalized();
      update();
    }
  }

  void mouseReleaseEvent(QMouseEvent *e) override {
    if (!m_dragging)
      return;
    m_dragging = false;
    QRect sel = QRect(m_dragStart, e->pos()).normalized();
    if (sel.width() >= 2 && sel.height() >= 2)
      m_boxes.append(toImageRect(sel));
    m_selection = {};
    update();
  }

public:
  explicit ImageCanvas(QPixmap pixmap, QString sourceFilename,
                       QWidget *parent = nullptr)
      : QWidget(parent), m_original(std::move(pixmap)),
        m_sourceFilename(std::move(sourceFilename)) {}

  void save() {
    auto path = QFileDialog::getSaveFileName(this, tr("Save Mask File"), {},
                                             tr("Mask Files (*.maskfile)"));
    if (path.isEmpty())
      return;
    if (!path.endsWith(".maskfile"))
      path += ".maskfile";

    MaskFile mf;
    mf.original_filename = m_sourceFilename;
    mf.image = m_original.toImage(); // original, unmasked
    mf.boxes = m_boxes;

    auto result = mf.save(path);
    if (result.is_error())
      QMessageBox::critical(this, tr("Save Failed"),
                            QString::fromStdString(result.error().message()));
  }
};

void editor_main() {
  auto filename = QFileDialog::getOpenFileName(
      nullptr, QFileDialog::tr("Open Image"), "/home",
      QFileDialog::tr("Image Files (*.png *.jpg *.bmp)"));
  if (filename.isEmpty())
    return;

  QPixmap pixmap(filename);
  if (pixmap.isNull())
    return;

  auto *window = new QMainWindow();
  window->setWindowTitle(filename);

  auto *canvas = new ImageCanvas(pixmap, filename);

  auto *toolbar = new QToolBar();
  toolbar->setMovable(false);
  auto *maskRectAction = toolbar->addAction("Mask Rect");
  maskRectAction->setCheckable(true);
  maskRectAction->setChecked(true);
  toolbar->addSeparator();
  toolbar->addAction("Save", [canvas]() { canvas->save(); });
  window->addToolBar(Qt::LeftToolBarArea, toolbar);

  window->setCentralWidget(canvas);

  QSize available = QApplication::primaryScreen()->availableSize() * 0.9;
  window->resize(pixmap.size().scaled(available, Qt::KeepAspectRatio));
  window->show();
}

} // namespace dndemask
