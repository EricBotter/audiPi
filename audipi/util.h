#ifndef UTIL_H
#define UTIL_H

#include <string>

namespace audipi {
    std::string &&left_pad_string(std::string &&str, size_t num, char paddingChar = ' ');
}

#endif //UTIL_H
