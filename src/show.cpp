#include "maskfile.hpp"
#include <QApplication>
#include <QFileDialog>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QScreen>
#include <QWidget>

namespace dndemask {

class ViewerCanvas : public QWidget {
  QPixmap m_original;
  QPixmap m_display;
  QVector<QRect> m_boxes; // in image coordinates

  QRect displayRect() const {
    return QRect((width() - m_display.width()) / 2,
                 (height() - m_display.height()) / 2,
                 m_display.width(), m_display.height());
  }

  QRect toDisplayBox(const QRect &imageBox) const {
    QRect dr = displayRect();
    double sx = (double)m_display.width() / m_original.width();
    double sy = (double)m_display.height() / m_original.height();
    return QRect(dr.left() + imageBox.x() * sx, dr.top() + imageBox.y() * sy,
                 imageBox.width() * sx, imageBox.height() * sy);
  }

  void updateDisplay() {
    m_display = m_original.scaled(size(), Qt::KeepAspectRatio,
                                  Qt::SmoothTransformation);
    update();
  }

protected:
  void resizeEvent(QResizeEvent *) override { updateDisplay(); }

  void paintEvent(QPaintEvent *) override {
    QPainter p(this);
    p.drawPixmap(displayRect().topLeft(), m_display);
    for (const QRect &box : m_boxes)
      p.fillRect(toDisplayBox(box), Qt::black);
  }

  void mousePressEvent(QMouseEvent *e) override {
    for (int i = 0; i < m_boxes.size(); ++i) {
      if (toDisplayBox(m_boxes[i]).contains(e->pos())) {
        QMenu menu;
        menu.addAction("Reveal", [this, i]() {
          m_boxes.removeAt(i);
          update();
        });
        menu.exec(e->globalPosition().toPoint());
        return;
      }
    }
  }

public:
  ViewerCanvas(QPixmap pixmap, QVector<QRect> boxes, QWidget *parent = nullptr)
      : QWidget(parent), m_original(std::move(pixmap)),
        m_boxes(std::move(boxes)) {}
};

void show_image(const QString &filename) {
  QPixmap pixmap;
  QVector<QRect> boxes;

  if (filename.endsWith(".maskfile", Qt::CaseInsensitive)) {
    auto result = MaskFile::load(filename);
    if (result.is_error())
      return;
    auto mf = std::move(*result);
    pixmap = QPixmap::fromImage(mf.image);
    boxes = mf.boxes;
  } else {
    pixmap = QPixmap(filename);
  }
  if (pixmap.isNull())
    return;

  QSize available = QApplication::primaryScreen()->availableSize() * 0.9;

  auto *canvas = new ViewerCanvas(pixmap, boxes);
  canvas->setWindowTitle(filename);
  canvas->resize(pixmap.size().scaled(available, Qt::KeepAspectRatio));
  canvas->show();
}

void viewer_main() {
  auto filename = QFileDialog::getOpenFileName(
      nullptr, QFileDialog::tr("Open Image"), "/home",
      QFileDialog::tr("Image Files (*.png *.jpg *.bmp *.maskfile)"));
  if (filename.isEmpty())
    return;
  show_image(filename);
}

} // namespace dndemask
