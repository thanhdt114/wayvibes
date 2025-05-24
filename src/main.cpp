#include <iostream>
#include <string>
#include <unordered_map>
#include <filesystem>
#include "audio.h"
#include "device.h"
#include "config.h"
#include <algorithm>

int main(int argc, char *argv[]) {
    std::string soundpackPath = "./";
    float volume = 1.0f;
    std::string configDir;
    const char *xdgConfigHome = std::getenv("XDG_CONFIG_HOME");
    configDir = (xdgConfigHome ? xdgConfigHome : std::string(getenv("HOME")) + "/.config") + "/wayvibes";

    if (!std::filesystem::exists(configDir)) {
        std::filesystem::create_directories(configDir);
    }

    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "--prompt") {
            saveInputDevice(configDir);
            return 0;
        } else if (std::string(argv[i]) == "-v" && (i + 1) < argc) {
            try {
                volume = std::stof(argv[i + 1]);
                i++;
            } catch (...) {
                std::cerr << "Invalid volume argument. Using default volume." << std::endl;
            }
        } else if (argv[i][0] != '-') {
            soundpackPath = argv[i];
        }
    }

    volume = std::clamp(volume, 0.0f, 10.0f);

    if (initializeAudioEngine() != MA_SUCCESS) {
        std::cerr << "Failed to initialize audio engine" << std::endl;
        return 1;
    }

    std::cout << "Soundpack: " << soundpackPath << std::endl;
    std::unordered_map<int, std::string> keySoundMap = loadKeySoundMappings(soundpackPath + "/config.json");

    std::string devicePath = getInputDevicePath(configDir);

    if (devicePath.empty()) {
        std::cout << "No device found. Prompting user." << std::endl;
        saveInputDevice(configDir);
        devicePath = getInputDevicePath(configDir);
    }

    runMainLoop(devicePath, keySoundMap, volume, soundpackPath);

    ma_engine_uninit(&engine);
    return 0;
}