#pragma once

#include "nm_classifier.hpp"
#include "nm_detector.hpp"
#include <QtWidgets/QLabel>
#include <QtWidgets>

class VideoLabelView : public QLabel {
public:
  explicit VideoLabelView(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
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
  void mousePressEvent(QMouseEvent *ev) override;
  void mouseMoveEvent(QMouseEvent *ev) override;
  void mouseReleaseEvent(QMouseEvent *ev) override;
};
