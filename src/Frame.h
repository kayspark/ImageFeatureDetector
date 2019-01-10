#pragma once
#include <chrono>
#include <opencv2/opencv.hpp>

class Frame {
  //	chrono::milliseconds timeStamp;
  cv::Mat img, mask;

public:
  //	Frame(chrono::milliseconds, Mat, Mat);
  Frame(const cv::Mat &pic, const cv::Mat &m)
      : img(pic)
      , mask(m) {}
  ~Frame() = default;
  cv::Mat getImg() const { return img; }
};