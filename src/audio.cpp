#define MINIAUDIO_IMPLEMENTATION
#include "audio.h"
#include "miniaudio.h"
#include <fcntl.h>
#include <iostream>
#include <linux/input.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <unistd.h>

ma_engine engine;

ma_result initializeAudioEngine() { return ma_engine_init(NULL, &engine); }

void uninitializeAudioEngine() { ma_engine_uninit(&engine); }

void playSound(const std::string &soundFile) {
  if (ma_engine_play_sound(&engine, soundFile.c_str(), NULL) != MA_SUCCESS) {
    std::cerr << "Error playing sound: " << soundFile << std::endl;
  }
}

void setVolume(float volume) { ma_engine_set_volume(&engine, volume); }

void runMainLoop(const std::string &devicePath,
                 const std::unordered_map<int, std::string> &keySoundMap, float volume,
                 const std::string &soundpackPath) {
  int fd = open(devicePath.c_str(), O_RDONLY | O_NONBLOCK);
  if (fd < 0) {
    std::cerr << "Failed to open input device: " << devicePath << std::endl;
    return;
  }

  setVolume(volume);

  std::cout << "Listening for key events on: " << devicePath << std::endl;

  struct input_event ev;
  while (true) {
    ssize_t n = read(fd, &ev, sizeof(ev));
    if (n == sizeof(ev)) {
      if (ev.type == EV_KEY && ev.value == 1) { // key press
        auto it = keySoundMap.find(ev.code);
        if (it != keySoundMap.end()) {
          std::string soundFile = soundpackPath + "/" + it->second;
          playSound(soundFile);
        }
      }
    } else {
      usleep(1000); // sleep 1ms to avoid busy loop
    }
  }

  close(fd);
}

void runMainLoopMulti(const std::string &keyboardDevicePath,
                     const std::string &mouseDevicePath,
                     const std::unordered_map<int, std::string> &keySoundMap,
                     float volume, const std::string &soundpackPath) {
  int kfd = -1, mfd = -1;

  if (!keyboardDevicePath.empty()) {
    kfd = open(keyboardDevicePath.c_str(), O_RDONLY | O_NONBLOCK);
    if (kfd < 0) {
      std::cerr << "Failed to open keyboard device: " << keyboardDevicePath << std::endl;
    } else {
      std::cout << "Listening for key events on: " << keyboardDevicePath << std::endl;
    }
  }

  if (!mouseDevicePath.empty()) {
    mfd = open(mouseDevicePath.c_str(), O_RDONLY | O_NONBLOCK);
    if (mfd < 0) {
      std::cerr << "Failed to open mouse device: " << mouseDevicePath << std::endl;
    } else {
      std::cout << "Listening for mouse events on: " << mouseDevicePath << std::endl;
    }
  }

  if (kfd < 0 && mfd < 0) {
    std::cerr << "No input devices available to listen on." << std::endl;
    return;
  }

  setVolume(volume);

  struct pollfd fds[2];
  int nfds = 0;

  if (kfd >= 0) {
    fds[nfds].fd = kfd;
    fds[nfds].events = POLLIN;
    nfds++;
  }

  if (mfd >= 0) {
    fds[nfds].fd = mfd;
    fds[nfds].events = POLLIN;
    nfds++;
  }

  struct input_event ev;
  while (true) {
    int ret = poll(fds, nfds, 50);  // 50ms timeout

    if (ret > 0) {
      for (int i = 0; i < nfds; ++i) {
        if (fds[i].revents & POLLIN) {
          int fd = fds[i].fd;
          ssize_t n = read(fd, &ev, sizeof(ev));

          if (n == sizeof(ev)) {
            if (ev.type == EV_KEY && ev.value == 1) {
              // Key or mouse button press
              auto it = keySoundMap.find(ev.code);
              if (it != keySoundMap.end()) {
                std::string soundFile = soundpackPath + "/" + it->second;
                playSound(soundFile);
              }
            }
          }
        }
      }
    }

    // Small sleep to avoid busy loop when no events
    usleep(1000);
  }

  if (kfd >= 0) {
    close(kfd);
  }

  if (mfd >= 0) {
    close(mfd);
  }
}