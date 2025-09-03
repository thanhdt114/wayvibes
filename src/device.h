#ifndef DEVICE_H
#define DEVICE_H

#include <string>
#include <unordered_map>

// find available keyboard devices
std::string findKeyboardDevices();

// find available mouse devices
std::string findMouseDevices();

// Run the main loop listening to both keyboard and mouse devices
void runMainLoopMulti(const std::string &keyboardDevicePath,
                      const std::string &mouseDevicePath,
                      const std::unordered_map<int, std::string> &keySoundMap,
                      float volume, const std::string &soundpackPath);

// get the input device path from the configuration directory
std::string getInputDevicePath(std::string &configDir);

// save the selected input device path to the configuration directory
void saveInputDevice(std::string &configDir);

// get/save mouse input device path
std::string getMouseDevicePath(std::string &configDir);
void saveMouseDevice(std::string &configDir);

#endif // DEVICE_H
