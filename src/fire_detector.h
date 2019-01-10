//
// Created by 박기수 on 2018-12-10.
//

#pragma once
#define _max_corners 10000
#include "ds.h"
#include "motionDetectionV1.h"
#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>

using _normal_pixel = cv::Point3_<uint8_t>;
using _short_pixel = uint8_t;
using _long_pixel = cv::Point3_<double>;
/* Find the minimal value of R G B */
template <typename T> T minrgb(const T r, const T g, const T b) { return (r <= g && r <= b) ? r : (g < b) ? g : b; }

class fire_detector {

public:
  explicit fire_detector(cv::Size &imgSize);
  const int get_bgm_frame_count() const { return _bgm_frame_count; }
  const double get_accumulate_weighted_alpha_bgm() const { return _accumulate_weighted_alpha_bgm; }
  const double get_accumulate_weighted_alpha_threshold() const { return _accumulate_weighted_alpha_threshold; }
  const int get_threshold_coefficient() const { return _threshold_coefficient; }

private:
  cv::Ptr<cv::Tracker> _tracker;
  const int _bgm_frame_count = 0;
  cv::Size _sizeWin;
  bool _init_tracker;
  cv::Rect2d _detected_area;

private:
  RectThrd _rect_thresh;

  const int _win_size{5};
  /* Processing Window Size (Frame) */
  const unsigned int _processing_windows = 15; // 15

  /* Background Model Update Coefficients */
  const double _accumulate_weighted_alpha_bgm = 0.1;
  const double _accumulate_weighted_alpha_threshold = 0.05;
  const int _threshold_coefficient = 5;

  /* Fire-like Region Threshold */
  const int _rect_width_threshold = 5;
  const int _rect_height_threshold = 5;
  const int _contour_area_threshold = 12;
  const int _contour_points_threshold = 12;
  /* Rect Motion */
  std::list<Centroid> _listCentroid;        // Centroid container
  std::vector<OFRect> vecOFRect;            // tmp container for ofrect
  std::multimap<int, OFRect> _mulMapOFRect; // BRect container
  std::vector<cv::Point2f> _featuresPrev;
  std::vector<cv::Point2f> _featuresCurr;

  // Pyramid Lucas-_max_corners
  std::vector<uchar> _featureFound;  //(_max_corners);
  std::vector<float> _featureErrors; //(_max_corners);
  std::vector<std::vector<cv::Point>> _contours;
  std::vector<cv::Vec4i> _hierachy;

  cv::Mat _maskMorphology;
  // for rgb image display copy from src
  cv::Mat _imgRGB;
  cv::Mat imgHSI;
  cv::Mat bufHSI;
  // mask rgb
  cv::Mat _maskRGB;
  cv::Mat maskHSI;

public:
  void detectFire(cv::Mat &maskMotion, motionDetectionV1 &bgs, cv::Mat &imgBackgroundModel,
                  cv::Mat &imgStandardDeviation, cv::Mat &img32FBackgroundModel, cv::Mat &img32FStandardDeviation,
                  cv::Mat &imgSrc, cv::Mat &imgGray, cv::Mat &imgDisplay);
  bool update_tracker(cv::Mat &img);
  void dilate(cv::Mat &mask);
  void findContours(cv::Mat &mask);
  bool checkContourPoints(Centroid &ctrd, const int thrdcp, const unsigned int pwindows);
  void motionOrientationHist(std::vector<Feature> &vecFeature, std::vector<unsigned int> &orient);

  double getEnergy(std::vector<Feature> &vecFeature, unsigned int &staticCount, unsigned int &totalPoints);

  bool checkContourEnergy(Centroid &ctrd, const unsigned int pwindows);

  void matchCentroid(cv::Mat &imgCenteroid, cv::Mat &img);
  /* get the feature points from contour
  input:
  imgDisplayCntr      : img for display contours
  imgDisplayFireRegion: img for boxing the fire-like region with rectangle
  contour             : after cvFindContour()
  trd                 : threshold
  output:
  vecOFRect           : fire-like obj will be assign to this container
  featuresPrev        : previous contours points
  featuresCurr        : current contours points
  return:
  the number of contour points:
  */
  int getContourFeatures(cv::Mat &img, cv::Mat &display);
  /* assign feature points to fire-like obj and then push to multimap
  input:
  vecOFRect:      fire-like obj
  status:			the feature stutas(found or not) after tracking
  featuresPrev:	previous feature points
  featuresCurr:   current feature points after tracking

  output:
  mulMapOFRect:	new candidate fire-like obj in current frame(with rectangle and
  motion vector information)

  */
  void calcOpticalFlow(cv::Mat &gray, cv::Mat &curr);
  void assignFeaturePoints();
  /* Counting the foldback point at each directions */
  void foldbackPoint(const std::vector<cv::Rect> &vecRect, DirectionsCount &count);
  /* Analysis the rect information */
  bool judgeDirectionsMotion(const std::vector<cv::Rect> &vecRect, cv::Rect &rectFire);
  /**
   *	@Purpose: check fire-like pixels by rgb model base on reference method
   *			  This function will change fire-like pixels to red
   *	@Parameter:
   *		frame: input source image
   *		mask: output mask
   */
  void checkByRGB(const cv::Mat &imgSrc, const cv::Mat &maskMotion, cv::Mat &maskRGB);
  /**
   *   @ Function: Convert RGB to HSI
   *   H: 0~360(degree)  HUE_R = 0 , HUE_G = 120 , HUE_B = 240
   *   S&I: 0~1
   *   @Parameter:       all img require same size
   *
   *                          [depth]           [channel]
   *		     imgRGB:     IPL_DEPTH_8U			 3
   *		     imgHSI:     IPL_DEPTH_64F			 3
   *		     maskRGB:    IPL_DEPTH_8U			 1
   */
  void RGB2HSIMask(cv::Mat &imgRGB, cv::Mat &imgHSI, cv::Mat &maskRGB);
  /**
   *	@Purpose: check fire-like pixels by rgb model base on reference method
   *			  This function will change fire-like pixels to red
   *	@Parameter:
   *		frame: input source image
   *		mask: output mask
   */
  void checkByHSI(cv::Mat &imgRGB, cv::Mat &imgHSI, cv::Mat &maskRGB, cv::Mat &maskHSI);
  /**
   *	@Function: markup the interest region based on mask
   *  @Parameter
   *		src: input image
   *		backup: output image (for display)
   *		mask: input mask
   */
  void regionMarkup(cv::Mat &imgSrc, cv::Mat &imgBackup, cv::Mat &mask);
  void setup_motion_model(const cv::Mat &maskMotion, cv::Mat &imgDisplay);
};
