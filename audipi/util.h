#ifndef UTIL_H
#define UTIL_H

#include <string>

#include "structs.h"

namespace audipi {
    std::string &&left_pad_string(std::string &&str, size_t num, char paddingChar = ' ');

    std::string msf_location_to_string(msf_location location);

    std::string msfs_location_to_string(msfs_location location);
}

#endif //UTIL_H
