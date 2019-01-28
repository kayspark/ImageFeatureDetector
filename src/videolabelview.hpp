#pragma once

#include "nm_classifier.hpp"
#include "nm_detector.hpp"
#include <QtWidgets/QLabel>
#include <QtWidgets>

class VideoLabelView : public QLabel {
public:
  explicit VideoLabelView(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags())
      : QLabel(parent, f) {}

  void resetUI() {

    if (!m_bandList.empty()) {
      for (auto &band : m_bandList) {
        band->hide();
        band = nullptr;
      }
      m_bandList.clear();
      m_rubberBand = nullptr;
    }
  }
  const std::vector<std::shared_ptr<QRubberBand>> &getBandList() const { return m_bandList; }

private:
  VideoLabelView() = default;
  bool m_mouse_pressed = false;
  QPoint mLastPoint;

  std::string m_data_file;
  std::string m_tracking_algorithm;
  std::unique_ptr<nm_classifier> m_nm_classifier;
  QPen m_pen;
  QPixmap mPixmap;
  std::unique_ptr<QPainter> mPainter;
  std::vector<std::shared_ptr<QRubberBand>> m_bandList;
  std::shared_ptr<QRubberBand> m_rubberBand;

protected:
  void mousePressEvent(QMouseEvent *ev) override {
    mLastPoint = ev->pos();
    m_mouse_pressed = true;
    m_rubberBand = std::make_shared<QRubberBand>(QRubberBand::Rectangle, this);
    QLabel::mousePressEvent(ev);
  }

  void mouseMoveEvent(QMouseEvent *ev) override {
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
  void mouseReleaseEvent(QMouseEvent *ev) override {
    // unsetCursor();
    m_mouse_pressed = false;
    if (m_rubberBand != nullptr)
      m_bandList.emplace_back(m_rubberBand);
    //_rubberBand->hide();
    QLabel::mouseReleaseEvent(ev);
  }
};
