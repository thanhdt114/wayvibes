#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <iostream>
#include <libevdev-1.0/libevdev/libevdev.h>
#include <string>
#include <unistd.h>
#include <vector>
#include <fstream>
#include "device.h"

std::string findKeyboardDevices() {
    DIR *dir = opendir("/dev/input");
    if (!dir) {
        std::cerr << "Failed to open /dev/input directory" << std::endl;
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
        std::cerr << "No input devices found!" << std::endl;
        return "";
    }

    std::vector<std::string> filteredDevices;

    std::cout << "Available Keyboard devices:" << std::endl;

    for (size_t i = 0, displayIndex = 1; i < devices.size(); ++i) {
        std::string devicePath = "/dev/input/" + devices[i];
        struct libevdev *dev = nullptr;
        int fd = open(devicePath.c_str(), O_RDONLY);
        if (fd < 0) {
            std::cerr << "Error opening " << devicePath << std::endl;
            continue;
        }

        int rc = libevdev_new_from_fd(fd, &dev);
        if (rc < 0) {
            std::cerr << "Failed to create evdev device for " << devicePath << std::endl;
            close(fd);
            continue;
        }

        if (libevdev_has_event_code(dev, EV_KEY, KEY_A)) {
            std::cout << displayIndex << ". " << libevdev_get_name(dev) << " (event"
                      << devices[i].substr(5) << ")" << std::endl;
            filteredDevices.push_back(devices[i]);
            displayIndex++;
        }

        libevdev_free(dev);
        close(fd);
    }

    if (filteredDevices.empty()) {
        std::cerr << "No suitable keyboard input devices found!" << std::endl;
        return "";
    }

    if (filteredDevices.size() == 1) {
        std::cout << "Selecting this keyboard device." << std::endl;
        return filteredDevices[0];
    }

    std::string selectedDevice;
    bool validChoice = false;

    while (!validChoice) {
        std::cout << "Select a keyboard input device (1-" << filteredDevices.size() << "): ";
        int choice;
        std::cin >> choice;

        if (choice >= 1 && choice <= filteredDevices.size()) {
            selectedDevice = filteredDevices[choice - 1];
            validChoice = true;
        } else {
            std::cerr << "Invalid choice. Please try again." << std::endl;
            exit(1);
        }
    }

    return selectedDevice;
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
    std::cout << "Please select a keyboard input device." << std::endl;
    std::string selectedDevice = findKeyboardDevices();
    if (!selectedDevice.empty()) {
        std::ofstream outputFile(configDir + "/input_device_path");
        outputFile << "/dev/input/" << selectedDevice;
        outputFile.close();
        std::cout << "Device path saved: /dev/input/" << selectedDevice << std::endl;
    } else {
        std::cerr << "No device selected. Exiting." << std::endl;
        exit(1);
    }
}