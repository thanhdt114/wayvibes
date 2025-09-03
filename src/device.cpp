#include "device.h"
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <libevdev-1.0/libevdev/libevdev.h>
#include <string>
#include <unistd.h>
#include <vector>

#define deviceDir "/dev/input/"
// ANSI color codes for terminal styling
#define RESET "\033[0m"
#define BOLD "\033[1m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define CYAN "\033[36m"

std::string findKeyboardDevices() {
  DIR *dir = opendir(deviceDir);
  if (!dir) {
    std::cerr << RED << "Failed to open /dev/input directory" << RESET << std::endl;
    return "";
  }

  std::vector<std::string> devices;
  struct dirent *entry;

  while ((entry = readdir(dir)) != NULL) {
    // if (!strstr(entry->d_name, "mouse")) {
    if (strncmp(entry->d_name, "event", 5) == 0) {
      devices.push_back(entry->d_name);
    }
  }

  closedir(dir);

  if (devices.empty()) {
    std::cerr << RED << "No input devices found!" << RESET << std::endl;
    return "";
  }

  std::vector<std::string> filteredDevices;

  std::cout << CYAN << "Available Keyboard devices:" << RESET << std::endl;

  for (size_t i = 0, displayIndex = 1; i < devices.size(); ++i) {
    std::string devicePath = deviceDir + devices[i];
    struct libevdev *dev = nullptr;
    int fd = open(devicePath.c_str(), O_RDONLY);
    if (fd < 0) {
      std::cerr << RED << "Error opening " << devicePath << RESET << std::endl;
      continue;
    }

    int rc = libevdev_new_from_fd(fd, &dev);
    if (rc < 0) {
      std::cerr << RED << "Failed to create evdev device for " << devicePath << RESET
                << std::endl;
      close(fd);
      continue;
    }

    if (libevdev_has_event_code(dev, EV_KEY, KEY_A)) {
      std::cout << CYAN << BOLD << displayIndex << ". " << RESET << YELLOW
                << libevdev_get_name(dev) << RESET << " (" << devices[i] << ")"
                << std::endl;
      filteredDevices.push_back(devices[i]);
      displayIndex++;
    }

    libevdev_free(dev);
    close(fd);
  }

  if (filteredDevices.empty()) {
    std::cerr << RED << "No suitable keyboard input devices found!" << RESET << std::endl;
    return "";
  }

  if (filteredDevices.size() == 1) {
    std::cout << CYAN << "Selecting this keyboard device." << RESET << std::endl;
    return filteredDevices[0];
  }

  std::string selectedDevice;
  bool validChoice = false;

  while (!validChoice) {
    std::cout << CYAN << "Select a keyboard input device (1-" << filteredDevices.size()
              << "): " << RESET;
    int choice;
    std::cin >> choice;

    if (choice >= 1 && choice <= filteredDevices.size()) {
      selectedDevice = filteredDevices[choice - 1];
      validChoice = true;
    } else {
      std::cerr << RED << "Invalid choice. Please try again." << RESET << std::endl;
    }
  }

  return selectedDevice;
}

std::string resolveToByIdPath(const std::string &eventDevice) {
  namespace fs = std::filesystem;
  std::string byIdDir = "/dev/input/by-id/";

  try {
    if (!fs::exists(byIdDir)) {
      return ""; // No by-id directory, fallback to event path
    }

    std::string targetPath = fs::canonical(deviceDir + eventDevice);

    for (const auto &entry : fs::directory_iterator(byIdDir)) {
      if (fs::is_symlink(entry)) {
        std::string linkTarget = fs::canonical(entry);

        if (linkTarget == targetPath) {
          return entry.path().string();
        }
      }
    }
  } catch (const std::exception &e) {
    std::cerr << RED << "Error resolving symlink: " << e.what() << RESET << std::endl;
  }

  return ""; // No matching symlink found
}

std::string getInputDevicePath(std::string &configDir) {
  std::string inputFilePath = configDir + "/input_device_path";
  std::ifstream inputFile(inputFilePath);

  if (inputFile.is_open()) {
    std::string devicePath;
    std::getline(inputFile, devicePath);
    inputFile.close();
    return devicePath;
  }

  return "";
}

void saveInputDevice(std::string &configDir) {
  std::string selectedDevice = findKeyboardDevices();
  if (!selectedDevice.empty()) {
    std::string byIdPath = resolveToByIdPath(selectedDevice);
    std::string deviceToSave;

    if (!byIdPath.empty()) {
      std::cout << GREEN << "\nUsing by-id path..." << RESET << std::endl;
      deviceToSave = byIdPath;
    } else {
      std::cout << YELLOW << BOLD
                << "\nNo by-id symlink found, using non-persistent event path..." << RESET
                << std::endl;
      deviceToSave = deviceDir + selectedDevice;
    }

    std::ofstream outputFile(configDir + "/input_device_path");
    outputFile << deviceToSave;
    outputFile.close();
    std::cout << GREEN << "Device path saved: " << deviceToSave << RESET << std::endl;
  } else {
    std::cerr << RED << "No device selected. Exiting." << RESET << std::endl;
    exit(1);
  }
}

// Find and select mouse devices (event interface)
std::string findMouseDevices() {
  DIR *dir = opendir(deviceDir);
  if (!dir) {
    std::cerr << RED << "Failed to open /dev/input directory" << RESET << std::endl;
    return "";
  }

  std::vector<std::string> devices;
  struct dirent *entry;

  while ((entry = readdir(dir)) != NULL) {
    if (strncmp(entry->d_name, "event", 5) == 0) {
      devices.push_back(entry->d_name);
    }
  }

  closedir(dir);

  if (devices.empty()) {
    std::cerr << RED << "No input devices found!" << RESET << std::endl;
    return "";
  }

  std::vector<std::string> filteredDevices;

  std::cout << CYAN << "Available Mouse devices:" << RESET << std::endl;

  for (size_t i = 0, displayIndex = 1; i < devices.size(); ++i) {
    std::string devicePath = deviceDir + devices[i];
    struct libevdev *dev = nullptr;
    int fd = open(devicePath.c_str(), O_RDONLY);
    if (fd < 0) {
      std::cerr << RED << "Error opening " << devicePath << RESET << std::endl;
      continue;
    }

    int rc = libevdev_new_from_fd(fd, &dev);
    if (rc < 0) {
      std::cerr << RED << "Failed to create evdev device for " << devicePath << RESET
                << std::endl;
      close(fd);
      continue;
    }

    // Identify mice by presence of REL_X/REL_Y or BTN_LEFT
    bool isMouse = libevdev_has_event_code(dev, EV_REL, REL_X) ||
                   libevdev_has_event_code(dev, EV_KEY, BTN_LEFT);
    if (isMouse) {
      std::cout << CYAN << BOLD << displayIndex << ". " << RESET << YELLOW
                << libevdev_get_name(dev) << RESET << " (" << devices[i] << ")"
                << std::endl;
      filteredDevices.push_back(devices[i]);
      displayIndex++;
    }

    libevdev_free(dev);
    close(fd);
  }

  if (filteredDevices.empty()) {
    std::cerr << RED << "No suitable mouse input devices found!" << RESET << std::endl;
    return "";
  }

  if (filteredDevices.size() == 1) {
    std::cout << CYAN << "Selecting this mouse device." << RESET << std::endl;
    return filteredDevices[0];
  }

  std::string selectedDevice;
  bool validChoice = false;

  while (!validChoice) {
    std::cout << CYAN << "Select a mouse input device (1-" << filteredDevices.size()
              << "): " << RESET;
    int choice;
    std::cin >> choice;

    if (choice >= 1 && choice <= filteredDevices.size()) {
      selectedDevice = filteredDevices[choice - 1];
      validChoice = true;
    } else {
      std::cerr << RED << "Invalid choice. Please try again." << RESET << std::endl;
    }
  }

  return selectedDevice;
}

std::string getMouseDevicePath(std::string &configDir) {
  std::string inputFilePath = configDir + "/mouse_input_device_path";
  std::ifstream inputFile(inputFilePath);

  if (inputFile.is_open()) {
    std::string devicePath;
    std::getline(inputFile, devicePath);
    inputFile.close();
    return devicePath;
  }

  return "";
}

void saveMouseDevice(std::string &configDir) {
  std::string selectedDevice = findMouseDevices();
  if (!selectedDevice.empty()) {
    std::string byIdPath = resolveToByIdPath(selectedDevice);
    std::string deviceToSave;

    if (!byIdPath.empty()) {
      std::cout << GREEN << "\nUsing by-id path..." << RESET << std::endl;
      deviceToSave = byIdPath;
    } else {
      std::cout << YELLOW << BOLD
                << "\nNo by-id symlink found, using non-persistent event path..." << RESET
                << std::endl;
      deviceToSave = deviceDir + selectedDevice;
    }

    std::ofstream outputFile(configDir + "/mouse_input_device_path");
    outputFile << deviceToSave;
    outputFile.close();
    std::cout << GREEN << "Mouse device path saved: " << deviceToSave << RESET
              << std::endl;
  } else {
    std::cerr << RED << "No mouse device selected. Continuing without mouse." << RESET
              << std::endl;
  }
}