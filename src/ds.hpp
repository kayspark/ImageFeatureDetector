#pragma once

#include <iostream>
#include <opencv2/opencv.hpp>

using RectThresh = struct RectThrd {
  int rectWidth;
  int rectHeight;
  int cntrArea;
};

/* for initialize rectThrd node */
inline RectThresh rectThrd(const int rectWidth, const int rectHeight, const int cntrArea) {
  RectThresh rt;

  rt.rectWidth = rectWidth;
  rt.rectHeight = rectHeight;
  rt.cntrArea = cntrArea;

  return rt;
}

/* Optical Flow feature points */
using Feature = struct Feature {
  cv::Point prev;
  cv::Point curr;
};

/* for initialize Feature node */
inline Feature feature(const cv::Point &prev, const cv::Point &curr) {
  Feature fr;

  fr.prev = prev;
  fr.curr = curr;

  return fr;
}

/* rect node(rect space) */
using OFRect = struct OFRect {
  bool match{};    // determine whether the rect is match or not
  int countCtrP{}; // the pixel count of contour
  // the pixel count of contour which is only be detected
  cv::Rect rect;                   // rect
  std::vector<Feature> vecFeature; // optical flow feature points
};

/* for initialize ofrect node */
inline OFRect ofRect(const cv::Rect &rect, const int countCtrP) {
  OFRect ofr;

  ofr.match = false;
  ofr.countCtrP = countCtrP;
  ofr.rect = rect;

  return ofr;
}

/* marker node */
using Centroid = struct Centroid {
  int countFrame{};                         // how many frame the centroid keeping in the region
  cv::Point centroid;                       // first detected centroid
  std::vector<cv::Rect> vecRect;            // rect information
  std::deque<std::vector<Feature>> dOFRect; // optical flow feature points
};

/* for initailize the new centroid node */
inline Centroid centroid(const OFRect &ofRect) {
  Centroid centroid1;

  centroid1.countFrame = 1; // first node
  centroid1.centroid = cv::Point(ofRect.rect.x + (ofRect.rect.width >> 1),
                                 ofRect.rect.y + (ofRect.rect.height >> 1)); // centroid position
  centroid1.vecRect.emplace_back(ofRect.rect);                               // push rect information
  centroid1.dOFRect.emplace_back(ofRect.vecFeature); // push contour optical flow feature(after optical
  // flow)

  return centroid1;
}

using DirectionsCount = struct DirectionsCount {
  int countUp;
  int countDown;
  int countLeft;
  int countRight;
};

inline void zeroCount(DirectionsCount &count) {
  count.countDown = count.countLeft = count.countRight = count.countUp = 0;
}

inline std::ostream &operator<<(std::ostream &os, const Centroid &ctrd) {
  os << "countFrame : " << ctrd.countFrame << std::endl;
  os << "x          : " << ctrd.centroid.x << std::endl;
  os << "y          : " << ctrd.centroid.y << std::endl;

  return os;
}

inline std::ostream &operator<<(std::ostream &os, const OFRect &ofr) {
  os << "match	: " << ofr.match << std::endl;
  os << "x		: " << ofr.rect.x << std::endl;
  os << "y		: " << ofr.rect.y << std::endl;
  os << "width	: " << ofr.rect.width << std::endl;
  os << "height	: " << ofr.rect.height << std::endl;
  os << "countCtrP: " << ofr.countCtrP << std::endl;

  return os;
}
