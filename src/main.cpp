#include "audio.h"
#include "config.h"
#include "device.h"
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <string>
#include <unistd.h>
#include <unordered_map>

void printHelp() {
  std::cout << "Usage: wayvibes [options] [soundpack_path]\n"
            << "Options:\n"
            << "  --device          Select input device\n"
            << "  -v <volume>       Set volume (0.0-10.0) (default: 1.0)\n"
            << "  --background, -bg Run in background (detached from terminal)\n"
            << "  --help, -h       Show this help message\n"
            << "Note: default soundpack path is './' (current directory) "
            << "Example: wayvibes ~/wayvibes/akko_lavender_purples/ -v 3" << std::endl;
}

int main(int argc, char *argv[]) {
  std::string soundpackPath = "./";
  float volume = 1.0f;
  std::string configDir;
  bool silent = false;
  const char *xdgConfigHome = std::getenv("XDG_CONFIG_HOME");
  configDir = (xdgConfigHome ? xdgConfigHome : std::string(getenv("HOME")) + "/.config") +
              "/wayvibes";

  if (!std::filesystem::exists(configDir)) {
    std::filesystem::create_directories(configDir);
  }

  for (int i = 1; i < argc; i++) {
    if (std::string(argv[i]) == "--device") {
      saveInputDevice(configDir);
      saveMouseDevice(configDir);
      return 0;
    } else if (std::string(argv[i]) == "-v" && (i + 1) < argc) {
      try {
        volume = std::stof(argv[i + 1]);
        i++;
      } catch (...) {
        std::cerr << "Invalid volume argument. Using default volume(1.0)." << std::endl;
      }
    } else if (std::string(argv[i]) == "--background" || std::string(argv[i]) == "-bg") {
      silent = true;
    } else if (std::string(argv[i]) == "--help" || std::string(argv[i]) == "-h") {
      printHelp();
      return 0;
    } else if (argv[i][0] != '-') {
      soundpackPath = argv[i];
    } else {
      std::cerr << "Unknown argument: " << argv[i] << std::endl;
      printHelp();
      return 1;
    }
  }

  if (silent) {
    pid_t pid = fork();
    if (pid < 0) {
      std::cerr << "Failed to fork for background mode." << std::endl;
      return 1;
    }
    if (pid > 0) {
      // Parent process exits, child continues in background
      return 0;
    }
    // Child process becomes session leader, detaches from terminal
    setsid();
    // Redirect stdio to /dev/null
    freopen("/dev/null", "r", stdin);
    freopen("/dev/null", "w", stdout);
    freopen("/dev/null", "w", stderr);
  }

  volume = std::clamp(volume, 0.0f, 10.0f);

  if (initializeAudioEngine() != MA_SUCCESS) {
    if (!silent) std::cerr << "Failed to initialize audio engine" << std::endl;
    return 1;
  }

  if (!silent) std::cout << "Soundpack: " << soundpackPath << std::endl;
  std::unordered_map<int, std::string> keySoundMap =
      loadKeySoundMappings(soundpackPath + "/config.json");

  std::string devicePath = getInputDevicePath(configDir);
  std::string mouseDevicePath = getMouseDevicePath(configDir);

  if (devicePath.empty()) {
    if (!silent) std::cout << "No device found. Prompting user." << std::endl;
    saveInputDevice(configDir);
    devicePath = getInputDevicePath(configDir);
  }

  if (mouseDevicePath.empty()) {
    if (!silent) std::cout << "No mouse device found. Prompting user." << std::endl;
    saveMouseDevice(configDir);
    mouseDevicePath = getMouseDevicePath(configDir);
  }

  runMainLoopMulti(devicePath, mouseDevicePath, keySoundMap, volume, soundpackPath);

  ma_engine_uninit(&engine);
  return 0;
}
