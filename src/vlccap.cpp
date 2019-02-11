#include "vlccap.hpp"
#include <cstring>
#include <string>
#include <thread>

vlc_capture::vlc_capture()
    : m_has_frame(false) {}

vlc_capture::vlc_capture(int width, int height)
    : vlc_capture() {
  m_size.width = width;
  m_size.height = height;
  mat_rgb.create(m_size, CV_8UC3);
}

vlc_capture::~vlc_capture() {
  try {
    release();
  } catch (...) {
    // TODO(kspark): log
  }
}

void vlc_capture::open(std::string url) {
  release();
  m_url = url;
  std::string arg_width = " --vmem-width=" + std::to_string(m_size.width);
  std::string arg_height = " --vmem-height=" + std::to_string(m_size.height);
  const char *args[] = {"-I",
                        "dummy",
                        "--no-audio",
                        "--network-caching=1000",
                        "--clock-jitter=0",
                        "--clock-synchro=0",
                        "--swscale-mode=2",
                        "--verbose=1",
                        arg_width.data(),
                        arg_height.data()};
  m_vlc_instance = libvlc_new(sizeof(args) / sizeof(args[0]), args);

  if (m_vlc_instance != nullptr) {
    libvlc_media_t *media = nullptr;
    media = libvlc_media_new_location(m_vlc_instance, m_url.data());
    if (media != nullptr) {
      m_media_player = libvlc_media_player_new_from_media(media);
      if (m_media_player != nullptr) {
        libvlc_media_release(media);
        libvlc_video_set_callbacks(m_media_player, &locker, &unlocker, nullptr, this);
        libvlc_video_set_format(m_media_player, "RV24", static_cast<unsigned int>(get_size().width),
                                static_cast<unsigned int>(get_size().height),
                                static_cast<unsigned int>(get_size().width * 3));

        int resp = libvlc_media_player_play(m_media_player);
        if (resp == 0) {
          is_open = true;
        } else {
          release();
        }
      }
    }
  } else {
    std::cout << "why??" << std::endl;
  }
}

void vlc_capture::release() {
  if (m_vlc_instance != nullptr) {
    libvlc_media_player_stop(m_media_player);
    libvlc_release(m_vlc_instance);
    libvlc_media_player_release(m_media_player);
    m_vlc_instance = nullptr;
    m_media_player = nullptr;
  }
  m_has_frame = false;
  is_open = false;
}

bool vlc_capture::isOpened() {
  if (!is_open) {
    return false;
  }

  libvlc_state_t state = libvlc_media_player_get_state(m_media_player);
  return (state != libvlc_Paused && state != libvlc_Stopped && state != libvlc_Ended && state != libvlc_Error);
}

bool vlc_capture::read(cv::Mat &outFrame) {
  while (!m_has_frame) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    if (!isOpened()) {
      return false; // connection closed
    }
  }

  {
    std::lock_guard<std::mutex> guard(m_mutex);
    outFrame = mat_rgb.clone();
    m_has_frame = true;
  }
  return true;
}

cv::Size vlc_capture::get_size() { return m_size; }

// whatever the choosen size was, need to alloc space for the whole input image
// here !

void *vlc_capture::lock(void **p_pixels) {
  // cout << "vlc_capture::lock" << endl;
  m_mutex.lock();
  *p_pixels = mat_rgb.data;
  return nullptr;
}

void vlc_capture::unlock(void * /*id*/, void *const * /*p_pixels*/) {
  m_has_frame = true;
  m_mutex.unlock();
}

void *vlc_capture::locker(void *data, void **p_pixels) { return static_cast<vlc_capture *>(data)->lock(p_pixels); }

void vlc_capture::unlocker(void *data, void *id, void *const *p_pixels) {
  static_cast<vlc_capture *>(data)->unlock(id, p_pixels);
}
