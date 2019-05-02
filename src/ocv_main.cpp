#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include "vlccap.hpp"
#include "NeuroMemEngine.hpp"
#include "nm_detector.hpp"
#include "nm_classifier.hpp"

void learnAbnormal(nm_classifier &classifier, cv::Mat &feature) { classifier.learn(feature); }

void loadKnowledges(nm_classifier &classifier) {
    int cnt = classifier.file_to_neurons();
    if (cnt > 0)
        std::cout << "loaded neuron count: " << cnt << std::endl;
}

void saveKnowledges(nm_classifier &classifier) { classifier.neurons_to_file(); }

void testKnowledge(nm_classifier &classifier, cv::Mat &feature) {
    uint16_t cat = classifier.classify(feature);
    if (cat < nm_classifier::UNKNOWN)
        cv::imshow("test Knowledge", feature);
    else
        std::cout << "test result is unknown" << std::endl;
}

void deleteKnowledge(nm_classifier &classifier, cv::Mat &feature) {
    if (classifier.deleteKnowledge(feature) < nm_classifier::UNKNOWN)
        std::cout << "delete best matched knowlege" << std::endl;
}

int main(int argc, char const *argv[]) {

#ifdef _WIN32
    using nm_device = NeuroMem::NeuroMemDevice;
    using nm_engine = NeuroMem::NeuroMemEngine;
    nm_device ds{};
    const uint16_t detected_device = nm_engine::GetDevices(&ds, 1);
#else
    nm_device ds;
    uint8_t detected_count = 10;
    uint16_t r = nm_get_devices(&ds, &detected_count);
#endif //_WIN32
    if (detected_count < 1) {
        std::cout << "NM devices are not connected. please make sure connection." << std::endl;
        std::cout << "Please refer http://www.theneuromorphic.com to get the device" << std::endl;
        system("pause");
        return (0);
    }

    vlc_capture cap(960, 544);
    nm_detector detector("dataset/cascade.xml", "CSRT");
    nm_classifier classifier;
    std::string url;
    if (argc > 1)
        url = argv[1];
    else {
        std::cout << "Usage: imagefeaturedetector rtsp_url" << std::endl;
        return 0;
    }
    std::vector<cv::Rect> rois;
    //  rois.push_back(cv::Rect(100, 100, 300, 300));
    std::string window_name = "test";
    cap.open(url);

    cv::Mat imgRT;
    cap.read(imgRT);
    cvtColor(imgRT, imgRT, cv::COLOR_BGR2RGB);
    rois.push_back(cv::selectROI(window_name, imgRT));
    if (rois.empty())
        return 0;

    while (cap.isOpened()) {
        cap.read(imgRT);
        if (!imgRT.empty()) {
            cvtColor(imgRT, imgRT, cv::COLOR_BGR2RGB);
            for_each(rois.begin(), rois.end(),
                     [&imgRT](const auto &r) { cv::rectangle(imgRT, r, cv::Scalar(128, 0, 0)); });
            cv::Mat gray;
            cv::cvtColor(imgRT, gray, cv::COLOR_BGR2GRAY);
            std::vector<cv::Rect> motions;
            detector.detect_candidate(gray, motions);
            for (const auto &motion : motions) {
                cv::Mat mat = gray(motion);

                const auto &band = std::find_if(rois.begin(), rois.end(),
                                                [&motion](const auto &b) { return (motion & b).area() > 0; });
                if (band != rois.end()) {
                    if (classifier.classify(mat) < nm_classifier::UNKNOWN) {
                        cv::rectangle(imgRT, motion, cv::Scalar(0, 0, 255));
                    } else {
                        cv::rectangle(imgRT, motion, cv::Scalar(0, 255, 0));
                    }
                } else {
                    cv::rectangle(imgRT, motion, cv::Scalar(0, 255, 255));
                }
            }
            cv::imshow(window_name, imgRT);
            if (cv::waitKey(5) == 27)
                break;
        }
    }
    return 0;
}
