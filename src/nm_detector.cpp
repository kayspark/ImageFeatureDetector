#include "nm_detector.h"
#include <opencv2/tracking.hpp>

using namespace cv;
using namespace std;

void nm_detector::createTrackerByName(const std::string &name) {
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

nm_detector::nm_detector(std::string &cascade, std::string &algorithm)
    : _scale(1.1), _tracker_algorithm(algorithm),
      detected_area(cv::Rect2d(0, 0, 0, 0)), initialized_tracker(false),
      objects(std::vector<cv::Rect>()),
      colors(std::array<cv::Scalar, 8>() =
                 {cv::Scalar(255, 0, 0), cv::Scalar(255, 128, 0),
                  cv::Scalar(255, 255, 0), cv::Scalar(0, 255, 0),
                  cv::Scalar(0, 128, 255), cv::Scalar(0, 255, 255),
                  cv::Scalar(0, 0, 255), cv::Scalar(255, 0, 255)}

      ) {
  if (cascade.empty())
    cascade = "./dataset/cascade.xml";
  if (!_cascade.load(cascade)) {
    std::cerr << "ERROR: Could not load classifier cascade" << std::endl;
    return;
  }
  createTrackerByName(_tracker_algorithm);
}

nm_detector::~nm_detector() {}
// detect roi
void nm_detector::detect_objects(cv::Mat &gray, cv::Mat &display) {
  double timer = 0;
  cv::Mat smallImg;

  // cvtColor(img, gray, COLOR_BGR2GRAY);
  double fx = 1 / _scale;
  resize(gray, smallImg, cv::Size(), fx, fx, cv::INTER_LINEAR_EXACT);
  // equalizeHist(smallImg, smallImg);

  timer = (double)cv::getTickCount();
  _cascade.detectMultiScale(smallImg, objects, _scale, 3,
                            0
                                //|CASCADE_FIND_BIGGEST_OBJECT
                                //|CASCADE_DO_ROUGH_SEARCH
                                | cv::CASCADE_SCALE_IMAGE,
                            Size(24, 24), Size(200, 200));

  timer = (double)getTickCount() - timer;
  std::cout << "suspicious object: " << objects.size()
            << " , time: " << timer * 1000 / getTickFrequency() << std::endl;
  int color_code = 0;
  if (objects.size() > 0) {
    detected_area = cv::Rect(
        Point(cvRound(objects[0].x * _scale), cvRound(objects[0].y * _scale)),
        Point(cvRound((objects[0].x + objects[0].width - 1) * _scale),
              cvRound((objects[0].y + objects[0].height - 1) * _scale)));
  } else
    detected_area = Rect2d(0, 0, 0, 0);

  draw_objects(display, color_code);

  // imshow( "result", img );
}
void nm_detector::draw_objects(const Mat &display, int color_code) const {
  for (const auto &r : objects) {
    Scalar color = colors[color_code++ % 8];
    rectangle(display, Point(cvRound(r.x * _scale), cvRound(r.y * _scale)),
              Point(cvRound((r.x + r.width - 1) * _scale),
                    cvRound((r.y + r.height - 1) * _scale)),
              color, 3, 8, 0);
  }
}
bool nm_detector::update_tracker(cv::Mat &gray, cv::Mat &display) {
  bool ret = !detected_area.empty();
  // initialized already
  if (initialized_tracker) {
    if (_tracker->update(gray, detected_area))
      cv::rectangle(display, detected_area, cv::Scalar(0, 0, 255), 3);
    else { // if cannot find
      detected_area = cv::Rect2d(0, 0, 0, 0);
      createTrackerByName(_tracker_algorithm);
      initialized_tracker = false;
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
      detect_objects(gray, display);
    }
  }

  // true if not empty
  return ret;
}
