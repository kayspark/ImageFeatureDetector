#pragma once

#include "motionCapture.hpp"
#include <opencv2/opencv.hpp>
//#include <opencv2/tracking/tracker.hpp>

class nm_detector {
  private:
    cv::CascadeClassifier m_cascade;
    //    cv::Ptr<cv::Tracker> m_tracker;
    bool initialized_tracker;
    std::vector<cv::Rect> objects;

  private:
    std::array<cv::Scalar, 8> colors;
    double m_scale;
    cv::Rect2d detected_area;
    std::string m_tracker_algorithm;
    double detection_time{};
    motionCapture m_motion;

  public:
    explicit nm_detector(std::string cascade, std::string algorithm);
    ~nm_detector() = default;
    //    void createTrackerByName(const std::string &name);
    //    bool update_tracker(cv::Mat &gray);
    void detect_candidate(cv::Mat &gray, std::vector<cv::Rect> &out);
    void detect_objects(const cv::Mat &gray);
    double get_detection_time() const { return detection_time; }
    bool validate_roi(cv::Mat &roi);
    cv::Rect2d get_detected() const { return detected_area; }
};
