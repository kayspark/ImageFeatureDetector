#pragma once
#include "Frame.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <chrono>
#include <mutex>
#include <vector>

class motionCapture {
  // cv::VideoCapture *_capture = nullptr;
  std::chrono::milliseconds _currentTime;
  const int _timeRange{3000}; // in milliseconds
  int _fps{0};
  std::vector<cv::Vec4i> _hierarchy;
  cv::TermCriteria _criteria;
  cv::Size _winSize;
  //  cv::Mat gray;
  cv::Mat _prevGray;
  cv::Mat _saved_mask;

  std::map<std::chrono::milliseconds, Frame> _frames;
  std::vector<std::map<std::chrono::milliseconds, std::vector<cv::Point>>> _allTracks;

public:
  void get_detected(std::vector<cv::Rect> &out);

private:
  cv::Ptr<cv::BackgroundSubtractor> _pBgs;
  void getFeaturePoints(const std::vector<cv::Point> &in, std::vector<cv::Point2f> &out);
  void uniteContours(std::vector<std::vector<cv::Point>> &cnts);

public:
  motionCapture();
  ~motionCapture();
  void find(cv::Mat &gray);
  void fill_tracks(std::vector<std::map<std::chrono::milliseconds, std::vector<cv::Point>>> &allTracks,
                   std::vector<std::vector<cv::Point>> &allContours) const;
};
