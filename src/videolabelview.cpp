#include "videolabelview.hpp"
#include <iostream>

VideoLabelView::VideoLabelView(QWidget *parent, Qt::WindowFlags f)
    : QLabel(parent, f) {}

void VideoLabelView::mousePressEvent(QMouseEvent *ev) {
  mLastPoint = ev->pos();
  m_mouse_pressed = true;
  m_rubberBand = std::make_shared<QRubberBand>(QRubberBand::Rectangle, this);
  QLabel::mousePressEvent(ev);
}

void VideoLabelView::mouseMoveEvent(QMouseEvent *ev) {
  if (m_mouse_pressed) {
    m_rubberBand->setGeometry(QRect(mLastPoint, ev->pos()).normalized());
    m_rubberBand->show();
    QString t = QString("(%1,%2), (%3,%4)")
                  .arg(mLastPoint.x())
                  .arg(mLastPoint.y())
                  .arg(m_rubberBand->size().width())
                  .arg(m_rubberBand->size().height());
    QToolTip::showText(ev->globalPos(), t, this);
    // std::cout << t.toStdString() << std::endl;
  }
  QLabel::mouseMoveEvent(ev);
}
inline void VideoLabelView::mouseReleaseEvent(QMouseEvent *ev) {

  // unsetCursor();
  m_mouse_pressed = false;
  if (m_rubberBand != nullptr)
    m_bandList.emplace_back(m_rubberBand);
  //_rubberBand->hide();
  QLabel::mouseReleaseEvent(ev);
}
