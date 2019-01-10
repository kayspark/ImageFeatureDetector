#if !defined(NM_DETECT_H)
#define NM_DETECT_H

#include "motionCapture.h"
#include <opencv2/opencv.hpp>
#include <opencv2/tracking/tracker.hpp>

class nm_detector {
private:
  cv::CascadeClassifier _cascade;
  cv::Ptr<cv::Tracker> _tracker;
  bool initialized_tracker;
  std::vector<cv::Rect> objects;
  std::vector<cv::Rect> candidate_objects;

public:
  const std::vector<cv::Rect> &get_candidate() const;

private:
  std::array<cv::Scalar, 8> colors;
  double _scale;
  cv::Rect2d detected_area;
  std::string _tracker_algorithm;
  double detection_time{};
  motionCapture _motion;

public:
  explicit nm_detector(std::string_view cascade, std::string_view algorithm);
  ~nm_detector() = default;
  void createTrackerByName(std::string_view name);
  bool update_tracker(cv::Mat &gray);
  void detect_candidate(cv::Mat &gray);
  void detect_objects(const cv::Mat &gray);
  double get_detection_time() const { return detection_time; }
  bool validate_roi(cv::Mat &roi);
  cv::Rect2d get_detected() const { return detected_area; }
};

#endif // NM_DETECT_H
