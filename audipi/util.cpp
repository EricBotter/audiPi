#include "util.h"

namespace audipi {
    // based on: https://stackoverflow.com/a/667219
    std::string &&left_pad_string(std::string &&str, const size_t num, const char paddingChar) {
        if (num > str.size())
            str.insert(0, num - str.size(), paddingChar);
        return std::move(str);
    }
}