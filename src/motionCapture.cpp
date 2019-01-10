#include "motionCapture.h"

using namespace std;
using namespace chrono;
using namespace cv;

motionCapture::motionCapture()
    : _pBgs(cv::createBackgroundSubtractorMOG2(10, 25, false))
    , _timeRange(3000)
    , _criteria(cv::TermCriteria::MAX_ITER + cv::TermCriteria::EPS, 20, 0.1)
    , _winSize(cv::Size(40, 40))
    , _fps(0) {
  // _capture = &cap;
}

motionCapture::~motionCapture() {
  //  _capture = nullptr;
  _pBgs.release();
}

void motionCapture::uniteContours(vector<vector<cv::Point>> &cnts) {
  bool isCrossed = true;
  vector<cv::Point> stub = {cv::Point(1, 1)};
  while (isCrossed) {
    isCrossed = false;
    for (auto i = cnts.begin(); i != cnts.end();) {
      if (i->size() < 30) {
        i = cnts.erase(i);
      } else {
        for (auto j = i; j != cnts.end(); j++) {
          if (i == j)
            continue;
          if ((boundingRect(*i) & boundingRect(*j)).width != 0) {
            i->insert(i->end(), j->begin(), j->end());
            *j = stub;
            isCrossed = true;
          }
        }
        i++;
      }
    }
  }
}

void motionCapture::getFeaturePoints(const std::vector<cv::Point> &in, std::vector<cv::Point2f> &out) {
  const int qty = 10;
  long step = (long)in.size() / qty;
  for (auto i = in.begin(); i < in.end(); i += step) {
    out.emplace_back(i->x, i->y);
  }
}

void motionCapture::find(Mat &gray) {
  if (gray.empty())
    return;
  Mat mask;
  Mat fgimg;
  _currentTime = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch());

  // resize(frame, frame, cv::Size(800, 600));
  _pBgs->apply(gray, mask, -1);

  mask.copyTo(_saved_mask);
  Frame frametoMap(gray, _saved_mask);

  _frames.emplace(_currentTime, frametoMap);
  fgimg = Scalar::all(0);
  // copy to fore ground image with mask;
  gray.copyTo(fgimg, mask);
  vector<vector<Point>> allContours;
  findContours(mask, allContours, _hierarchy, RETR_EXTERNAL, CHAIN_APPROX_NONE);
  if (!allContours.empty() && allContours.size() < 1000) {
    gray.copyTo(gray);
    //  cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

    uniteContours(allContours);

    if (_prevGray.empty()) {
      gray.copyTo(_prevGray);
    }
    if (_allTracks.empty()) {
      fill_tracks(_allTracks, allContours);
    } else {
      vector<uchar> status;
      vector<float> err;
      multimap<int, int> pointsNum;
      vector<Point2f> pointsPrev, pointsNow;
      long trackNumber = 0;
      for (const auto &track : _allTracks) {
        if (!track.empty()) {
          vector<Point2f> tmpVec;
          getFeaturePoints(track.rbegin()->second, tmpVec);
          for (auto i = pointsPrev.size(); i < pointsPrev.size() + tmpVec.size(); i++) {
            pointsNum.emplace(trackNumber, i);
          }
          pointsPrev.insert(pointsPrev.end(), tmpVec.begin(), tmpVec.end());
          trackNumber++;
        }
      }
      calcOpticalFlowPyrLK(_prevGray, gray, pointsPrev, pointsNow, status, err, _winSize, 3, _criteria, 0, 0.001);
      trackNumber = 0;
      for (auto &track : _allTracks) {
        if (!track.empty()) {
          vector<Point2f> tmpVecPoints;
          const auto &[start, end] = pointsNum.equal_range(static_cast<const int>(trackNumber));
          for_each(start, end,
                   [&tmpVecPoints, &pointsNow](const auto &it) { tmpVecPoints.emplace_back(pointsNow[it.second]); });
          Rect tmpRect = boundingRect(tmpVecPoints);
          milliseconds cur_time = _currentTime;
          allContours.erase(std::remove_if(allContours.begin(), allContours.end(),
                                           [&tmpRect, &cur_time, &track](const auto &contour) {
                                             bool ret = false;
                                             Rect rect = boundingRect(contour);
                                             if ((tmpRect & rect).height > 10 && (tmpRect & rect).width > 10) {
                                               track.emplace(cur_time, contour);
                                               ret = true;
                                             }
                                             return ret;
                                           }),
                            allContours.end());
        }
        trackNumber++;
      }
      fill_tracks(_allTracks, allContours);
    }
  }
  milliseconds endtime = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch());
  _fps = static_cast<int>(1000.0 / (endtime - _currentTime).count());
  swap(_prevGray, gray);
  //   display();
}
void motionCapture::fill_tracks(vector<map<milliseconds, vector<Point>>> &allTracks,
                                vector<vector<Point>> &allContours) const {
  milliseconds cur_time = _currentTime;
  for_each(allContours.begin(), allContours.end(), [&allTracks, &cur_time](const vector<Point> &cnt) {
    map<milliseconds, vector<Point>> oneTrack;
    oneTrack.emplace(cur_time, cnt);
    allTracks.emplace_back(oneTrack);
  });
}

void motionCapture::display() {
  auto frameIt = _frames.rbegin();
  milliseconds time = frameIt->first;
  Mat outFrame;
  outFrame = frameIt->second.getImg();
  int cnt = 0;
  for (const auto &track : _allTracks) {
    if (track.size() > 1) {
      auto mapIt = track.find(time);
      if (mapIt == track.end())
        continue;
      Rect r = boundingRect(mapIt->second);
      rectangle(outFrame, r, Scalar(0, 255, 0), 3, 8, 0);
      stringstream ss;
      ss << cnt++;
      string stringNumber = ss.str();
      putText(outFrame, stringNumber, Point(r.x + 5, r.y + 5), FONT_HERSHEY_COMPLEX_SMALL, 1, Scalar(0, 0, 255), 1, 8);
    }
  }
  stringstream sst;
  sst << _fps;
  string fpsString = "FPS = " + sst.str();
  putText(outFrame, fpsString, Point(20, outFrame.rows - 20), FONT_HERSHEY_COMPLEX_SMALL, 0.8, Scalar(255, 0, 255), 1,
          8);
  imshow("tracks", outFrame);
  waitKey(5);
}
void motionCapture::get_detected(std::vector<cv::Rect> &out) const {
  if (!out.empty())
    out.clear();
  auto frameIt = _frames.rbegin();
  milliseconds time = frameIt->first;
  Mat outFrame;
  outFrame = frameIt->second.getImg();
  int cnt = 0;
  for (const auto &track : _allTracks) {
    if (track.size() > 1) {
      auto mapIt = track.find(time);
      if (mapIt == track.end())
        continue;
      out.emplace_back(boundingRect(mapIt->second));
      // rectangle(outFrame, r, Scalar(0, 255, 0), 3, 8, 0);
    }
  }
}
