#include "nm_detector.h"
#include "motionCapture.h"
#include <opencv2/tracking.hpp>

using namespace cv;
using namespace std;

void nm_detector::createTrackerByName(const std::string_view name) {
  if (name == "KCF")
    _tracker = cv::TrackerKCF::create();
  else if (name == "TLD")
    _tracker = cv::TrackerTLD::create();
  else if (name == "BOOSTING")
    _tracker = cv::TrackerBoosting::create();
  else if (name == "MEDIAN_FLOW")
    _tracker = cv::TrackerMedianFlow::create();
  else if (name == "MIL")
    _tracker = cv::TrackerMIL::create();
  else if (name == "GOTURN")
    _tracker = cv::TrackerGOTURN::create();
  else if (name == "MOSSE")
    _tracker = cv::TrackerMOSSE::create();
  else if (name == "CSRT")
    _tracker = cv::TrackerCSRT::create();
  else
    CV_Error(cv::Error::StsBadArg, "Invalid tracking algorithm name\n");
}

nm_detector::nm_detector(std::string_view cascade, std::string_view algorithm)
    : _scale(1.1)
    , _tracker_algorithm(algorithm)
    , detected_area(cv::Rect2d(0, 0, 0, 0))
    , initialized_tracker(false)
    , colors(std::array<cv::Scalar, 8>() = {cv::Scalar(255, 0, 0), cv::Scalar(255, 128, 0), cv::Scalar(255, 255, 0),
                                            cv::Scalar(0, 255, 0), cv::Scalar(0, 128, 255), cv::Scalar(0, 255, 255),
                                            cv::Scalar(0, 0, 255), cv::Scalar(255, 0, 255)}

      ) {
  if (cascade.empty())
    cascade = "./dataset/cascade.xml";
  if (!_cascade.load(cascade.data())) {
    std::cerr << "ERROR: Could not load classifier cascade" << std::endl;
    return;
  }
  createTrackerByName(_tracker_algorithm);
}

bool nm_detector::validate_roi(cv::Mat &roi) {
  bool ret = false;
  cv::Mat smallImg;
  std::vector<cv::Rect> t_objects;
  // cvtColor(img, gray, COLOR_BGR2GRAY);
  double fx = 1.1;
  resize(roi, smallImg, cv::Size(), fx, fx, cv::INTER_LINEAR_EXACT);
  // equalizeHist(smallImg, smallImg);

  _cascade.detectMultiScale(smallImg, t_objects, 1.1, 3,
                            0
                              //|CASCADE_FIND_BIGGEST_OBJECT
                              //|CASCADE_DO_ROUGH_SEARCH
                              | cv::CASCADE_SCALE_IMAGE,
                            Size(24, 24), Size(200, 200));

  if (!objects.empty()) {
    ret = true;
  }
  return ret;
  // draw_objects(display, color_code);
  // imshow( "result", img );
} // detect roi

void nm_detector::detect_candidate(Mat &gray) {
  auto timer = (double)cv::getTickCount();
  _motion.find(gray);
  detection_time = ((double)getTickCount() - timer) * 1000 / getTickFrequency();
  _motion.get_detected(candidate_objects);
}

void nm_detector::detect_objects(const cv::Mat &gray) {
  auto timer = (double)cv::getTickCount();
  // for (const auto &r : candidate_objects) {
  cv::Mat smallImg; // = gray;//(r);
  // cvtColor(img, gray, COLOR_BGR2GRAY);
  double fx = 1 / _scale;
  resize(gray, smallImg, cv::Size(), fx, fx, cv::INTER_LINEAR_EXACT);
  // equalizeHist(smallImg, smallImg);
  _cascade.detectMultiScale(smallImg, objects, _scale, 3,
                            0
                              //|CASCADE_FIND_BIGGEST_OBJECT
                              //|CASCADE_DO_ROUGH_SEARCH
                              | cv::CASCADE_SCALE_IMAGE,
                            Size(24, 24), Size(200, 200));
  //}
  detection_time = ((double)getTickCount() - timer) * 1000 / getTickFrequency();
  //  std::cout << "suspicious object: " << objects.size()
  //            << " , time: " << timer * 1000 / getTickFrequency() <<
  //            std::endl;
  std::for_each(objects.begin(), objects.end(), [&](auto &rect) {
    rect.x = rect.x * _scale;
    rect.y = rect.y * _scale;
    rect.width = rect.width * _scale;
    rect.height = rect.height * _scale;
    // rectangle(display, rect, colors[0], 3);
  });
  if (!objects.empty()) {
    detected_area = objects[0];
  } else
    detected_area = Rect2d(0, 0, 0, 0);
}

bool nm_detector::update_tracker(cv::Mat &gray) {
  bool ret = !objects.empty();
  // initialized already
  detect_candidate(gray);
  if (initialized_tracker) {
    if (_tracker->update(gray, detected_area)) {
      /* cv::Mat roi = gray(detected_area);
     if (!validate_roi(roi)) {
        detected_area = cv::Rect2d(0, 0, 0, 0);
        initialized_tracker = false;
        ret = false;
     }*/
      // cv::rectangle(display, detected_area, cv::Scalar(0, 0, 255), 3);
    } else {
      detected_area = cv::Rect2d(0, 0, 0, 0);
      createTrackerByName(_tracker_algorithm);
      initialized_tracker = false;
      ret = false;
    }
  } else {
    if (ret) { // detected area
      // initializes the tracker
      if (!_tracker->init(gray, detected_area)) {
        detected_area = cv::Rect2d(0, 0, 0, 0);
        std::cout << "***Could not initialize tracker...***\n";
        return ret;
      }
      initialized_tracker = true;
    } else {
      initialized_tracker = false;
      detected_area = cv::Rect2d(0, 0, 0, 0);
      // detect_objects(gray);
    }
  }
  // true if not empty
  return ret;
}
const vector<Rect> &nm_detector::get_candidate() const { return candidate_objects; }
