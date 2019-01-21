#pragma once

#include "vlc/vlc.h"
#include <atomic>
#include <mutex>
#include <opencv2/opencv.hpp>

class vlc_capture {
public:
  vlc_capture(int width, int height);
  ~vlc_capture();

  void open(std::string_view url);
  void release();
  bool isOpened();
  bool read(cv::Mat &outFrame);
  cv::Size get_size();

private:
  vlc_capture();
  void *lock(void **p_pixels);
  void unlock(void *id, void *const *p_pixels);

  static void *locker(void *data, void **p_pixels);
  static void unlocker(void *data, void *id, void *const *p_pixels);

private:
  std::mutex _mutex;
  std::string_view _url;
  cv::Mat _rgb;
  bool _is_open{false};
  std::atomic<bool> _has_frame;
  libvlc_instance_t *_vlc_instance = nullptr;
  libvlc_media_player_t *_media_player = nullptr;
  cv::Size _size;
};
