#include <utility>

#include "motionCapture.hpp"
#include "nm_detector.hpp"
#include <opencv2/tracking.hpp>

using namespace cv;
using namespace std;

// shamelessly copied from opencv tracker sample codes
// all of them seems not adaquate for production use for now
// CNN based tracker like SSD will be merged soon.
void nm_detector::createTrackerByName(const std::string &name) {
    if (name == "KCF")
        m_tracker = cv::TrackerKCF::create();
    else if (name == "TLD")
        m_tracker = cv::TrackerTLD::create();
    else if (name == "BOOSTING")
        m_tracker = cv::TrackerBoosting::create();
    else if (name == "MEDIAN_FLOW")
        m_tracker = cv::TrackerMedianFlow::create();
    else if (name == "MIL")
        m_tracker = cv::TrackerMIL::create();
    else if (name == "GOTURN")
        m_tracker = cv::TrackerGOTURN::create();
    else if (name == "MOSSE")
        m_tracker = cv::TrackerMOSSE::create();
    else if (name == "CSRT")
        m_tracker = cv::TrackerCSRT::create();
    else
        std::cerr << "Invalid tracking algorithm name\n";
}

nm_detector::nm_detector(std::string cascade, std::string algorithm)
    : m_scale(1.1)
    , m_tracker_algorithm(std::move(algorithm))
    , detected_area(cv::Rect2d(0, 0, 0, 0))
    , initialized_tracker(false)
    , colors(std::array<cv::Scalar, 8>() = {cv::Scalar(255, 0, 0), cv::Scalar(255, 128, 0), cv::Scalar(255, 255, 0),
                                            cv::Scalar(0, 255, 0), cv::Scalar(0, 128, 255), cv::Scalar(0, 255, 255),
                                            cv::Scalar(0, 0, 255), cv::Scalar(255, 0, 255)}) {
    if (cascade.empty()) {
        cascade = ":/dataset/cascade.xml";
    }
    if (!m_cascade.load(cascade)) {
        std::cerr << "ERROR: Could not load classifier cascade" << std::endl;
        return;
    }
    createTrackerByName(m_tracker_algorithm);
}

bool nm_detector::validate_roi(cv::Mat &roi) {
    bool ret = false;
    cv::Mat smallImg;
    std::vector<cv::Rect> t_objects;
    // cvtColor(img, gray, COLOR_BGR2GRAY);
    double fx = 1.1;
    resize(roi, smallImg, cv::Size(), fx, fx, cv::INTER_LINEAR_EXACT);
    // equalizeHist(smallImg, smallImg);
    m_cascade.detectMultiScale(smallImg, t_objects, 1.1, 3,
                               0
                                   //|CASCADE_FIND_BIGGEST_OBJECT
                                   //|CASCADE_DO_ROUGH_SEARCH
                                   | cv::CASCADE_SCALE_IMAGE,
                               Size(24, 24), Size(200, 200));

    if (!objects.empty()) {
        ret = true;
    }
    return ret;
    // imshow( "result", img );
} // detect roi

void nm_detector::detect_candidate(Mat &gray, std::vector<cv::Rect> &out) {
    auto timer = static_cast<double>(cv::getTickCount());
    m_motion.find(gray);
    detection_time = (static_cast<double>(getTickCount()) - timer) * 1000 / getTickFrequency();
    m_motion.get_detected(out);
}

void nm_detector::detect_objects(const cv::Mat &gray) {
    auto timer = static_cast<double>(cv::getTickCount());
    cv::Mat smallImg; // = gray;//(r);
    // cvtColor(img, gray, COLOR_BGR2GRAY);
    double fx = 1 / m_scale;
    resize(gray, smallImg, cv::Size(), fx, fx, cv::INTER_LINEAR_EXACT);
    m_cascade.detectMultiScale(smallImg, objects, m_scale, 3,
                               0
                                   //|CASCADE_FIND_BIGGEST_OBJECT
                                   //|CASCADE_DO_ROUGH_SEARCH
                                   | cv::CASCADE_SCALE_IMAGE,
                               Size(24, 24), Size(200, 200));
    //}
    detection_time = (static_cast<double>(getTickCount()) - timer) * 1000 / getTickFrequency();
    //  std::cout << "suspicious object: " << objects.size()
    //            << " , time: " << timer * 1000 / getTickFrequency() <<
    //            std::endl;
    std::for_each(objects.begin(), objects.end(), [&](auto &rect) {
        rect.x = rect.x * m_scale;
        rect.y = rect.y * m_scale;
        rect.width = rect.width * m_scale;
        rect.height = rect.height * m_scale;
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
    if (initialized_tracker) {
        if (m_tracker->update(gray, detected_area)) {
            /* cv::Mat roi = gray(detected_area);
           if (!validate_roi(roi)) {
              detected_area = cv::Rect2d(0, 0, 0, 0);
              initialized_tracker = false;
              ret = false;
           }*/
            // cv::rectangle(display, detected_area, cv::Scalar(0, 0, 255), 3);
        } else {
            detected_area = cv::Rect2d(0, 0, 0, 0);
            createTrackerByName(m_tracker_algorithm);
            initialized_tracker = false;
            ret = false;
        }
    } else {
        if (ret) { // detected area
            // initializes the tracker
            if (!m_tracker->init(gray, detected_area)) {
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
