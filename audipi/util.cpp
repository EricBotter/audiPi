#include "util.h"

namespace audipi {
    // based on: https://stackoverflow.com/a/667219
    std::string &&left_pad_string(std::string &&str, const size_t num, const char paddingChar) {
        if (num > str.size())
            str.insert(0, num - str.size(), paddingChar);
        return std::move(str);
    }

    std::string msf_location_to_string(const msf_location location) {
        return left_pad_string(std::to_string(+location.minute), 2, '0')
               + ":" + left_pad_string(std::to_string(+location.second), 2, '0')
               + ":" + left_pad_string(std::to_string(+location.frame), 2, '0');
    }

    std::string msfs_location_to_string(const msfs_location location) {
        return msf_location_to_string(location)
               + "." + left_pad_string(std::to_string(+location.samples), 3, '0');
    }
}
