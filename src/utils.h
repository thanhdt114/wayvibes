#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <iostream>

namespace Utils {
    void handleError(const std::string &message) {
        std::cerr << "Error: " << message << std::endl;
        exit(1);
    }

    void logMessage(const std::string &message) {
        std::cout << message << std::endl;
    }
}

#endif // UTILS_H