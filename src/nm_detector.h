#if !defined(NM_DETECT_H)
#define NM_DETECT_H

#include <opencv2/opencv.hpp>
#include <opencv2/tracking/tracker.hpp>

class nm_detector {
private:
  cv::CascadeClassifier _cascade;
  cv::Ptr<cv::Tracker> _tracker;
  bool initialized_tracker;
  std::vector<cv::Rect> objects;
  std::array<cv::Scalar, 8> colors;
  double _scale;
  cv::Rect2d detected_area;
  std::string _tracker_algorithm;

public:
  explicit nm_detector(std::string &cascade, std::string &algorithm);
  ~nm_detector();
  void createTrackerByName(const std::string &name);
  bool update_tracker(cv::Mat &gray, cv::Mat &display);
  void detect_objects(cv::Mat &gray, cv::Mat &display);
  void draw_objects(const cv::Mat &display, int color_code) const;
};

#endif // NM_DETECT_H