#pragma once

#include "vlc/vlc.h"
#include <atomic>
#include <mutex>
#include <opencv2/opencv.hpp>

class vlc_capture {
public:
  vlc_capture(int width, int height);
  ~vlc_capture();

  void open(std::string &url);
  void release();
  bool isOpened();
  bool read(cv::Mat &outFrame);
  cv::Size get_size();
  vlc_capture();

private:
  void *lock(void **p_pixels);
  void unlock(void *id, void *const *p_pixels);

  static void *locker(void *data, void **p_pixels);
  static void unlocker(void *data, void *id, void *const *p_pixels);

private:
  std::mutex m_mutex;
  std::string m_url;
  cv::Mat mat_rgb;
  bool is_open{false};
  std::atomic<bool> m_has_frame;
  libvlc_instance_t *m_vlc_instance = nullptr;
  libvlc_media_player_t *m_media_player = nullptr;
  cv::Size m_size;
};
