#ifndef FRAME_HPP
#define FRAME_HPP
#include <chrono>
#include <opencv2/opencv.hpp>
#include <utility>

class Frame {
    //	chrono::milliseconds timeStamp;
    cv::Mat img, mask;

  public:
    //	Frame(chrono::milliseconds, Mat, Mat);
    Frame(cv::Mat pic, cv::Mat m)
        : img(std::move(pic))
        , mask(std::move(m)) {}
    ~Frame() = default;
    cv::Mat getImg() const { return img; }
};
#endif // FRAME_HPP
