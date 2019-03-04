#pragma once
#include "Frame.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <chrono>
#include <mutex>
#include <vector>

class motionCapture {
  // cv::VideoCapture *_capture = nullptr;
  std::chrono::milliseconds m_currentTime;
  const int m_timeRange{3000}; // in milliseconds
  int m_fps{0};
  std::vector<cv::Vec4i> m_hierarchy;
  cv::TermCriteria m_criteria;
  cv::Size m_winSize;
  cv::Mat m_prevGray;
  cv::Mat m_saved_mask;

  std::map<std::chrono::milliseconds, Frame> m_frames;
  std::vector<std::map<std::chrono::milliseconds, std::vector<cv::Point>>> m_allTracks;

public:
  void get_detected(std::vector<cv::Rect> &out);

private:
  cv::Ptr<cv::BackgroundSubtractor> m_pBgs;
  void getFeaturePoints(const std::vector<cv::Point> &in, std::vector<cv::Point2f> &out);
  void uniteContours(std::vector<std::vector<cv::Point>> &cnts);

public:
  motionCapture();
  ~motionCapture();
  void find(cv::Mat &gray);
  void fill_tracks(std::vector<std::map<std::chrono::milliseconds, std::vector<cv::Point>>> &allTracks,
                   std::vector<std::vector<cv::Point>> &allContours) const;
};
