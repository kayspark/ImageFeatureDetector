#include "vlccap.hpp"
#include <cstring>
#include <string_view>
#include <thread>

vlc_capture::vlc_capture()
    : 
     _has_frame(false) {}

vlc_capture::vlc_capture(int width, int height)
    : vlc_capture() {
  _size.width = width;
  _size.height = height;
  _rgb.create(_size, CV_8UC3);
}

vlc_capture::~vlc_capture() {
  try {
    release();
  } catch (...) {
    // TODO(kspark): log
  }
}

void vlc_capture::open(std::string_view url) {
  release();
  _url = url;
  std::string_view arg_width = " --vmem-width=" + std::to_string(_size.width);
  std::string_view arg_height = " --vmem-height=" + std::to_string(_size.height);
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
  _vlc_instance = libvlc_new(sizeof(args) / sizeof(args[0]), args);

  if (_vlc_instance != nullptr) {
    libvlc_media_t *media = nullptr;
    media = libvlc_media_new_location(_vlc_instance, _url.data());
    if (media != nullptr) {
      _media_player = libvlc_media_player_new_from_media(media);
      if (_media_player != nullptr) {
        libvlc_media_release(media);
        libvlc_video_set_callbacks(_media_player, &locker, &unlocker, nullptr, this);
        libvlc_video_set_format(_media_player, "RV24", static_cast<unsigned int>(get_size().width),
                                static_cast<unsigned int>(get_size().height),
                                static_cast<unsigned int>(get_size().width * 3));

        int resp = libvlc_media_player_play(_media_player);
        if (resp == 0) {
          _is_open = true;
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
  if (_vlc_instance != nullptr) {
    libvlc_media_player_stop(_media_player);
    libvlc_release(_vlc_instance);
    libvlc_media_player_release(_media_player);
    _vlc_instance = nullptr;
    _media_player = nullptr;
  }
  _has_frame = false;
  _is_open = false;
}

bool vlc_capture::isOpened() {
  if (!_is_open) {
    return false;
}

  libvlc_state_t state = libvlc_media_player_get_state(_media_player);
  return (state != libvlc_Paused && state != libvlc_Stopped && state != libvlc_Ended && state != libvlc_Error);
}

bool vlc_capture::read(cv::Mat &outFrame) {
  while (!_has_frame) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    if (!isOpened()) {
      return false; // connection closed
}
  }

  {
    std::lock_guard<std::mutex> guard(_mutex);
    outFrame = _rgb.clone();
    _has_frame = true;
  }
  return true;
}

cv::Size vlc_capture::get_size() { return _size; }

// whatever the choosen size was, need to alloc space for the whole input image
// here !

void *vlc_capture::lock(void **p_pixels) {
  // cout << "vlc_capture::lock" << endl;
  _mutex.lock();
  *p_pixels = _rgb.data;
  return nullptr;
}

void vlc_capture::unlock(void * /*id*/, void *const * /*p_pixels*/) {
  _has_frame = true;
  _mutex.unlock();
}

void *vlc_capture::locker(void *data, void **p_pixels) { return static_cast<vlc_capture *>(data)->lock(p_pixels); }

void vlc_capture::unlocker(void *data, void *id, void *const *p_pixels) {
  static_cast<vlc_capture *>(data)->unlock(id, p_pixels);
}
