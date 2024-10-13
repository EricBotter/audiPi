#ifndef STRUCTS_H
#define STRUCTS_H
#include <vector>
#include <sys/types.h>

namespace audipi {
    struct msf_location {
        u_int8_t minute;
        u_int8_t second;
        u_int8_t frame;
    };

    msf_location operator-(const msf_location& left, const msf_location& right);
    msf_location operator+(const msf_location& left, const msf_location& right);

    struct audio_disk_toc_entry {
        u_int8_t track_num;
        msf_location address;
        msf_location duration; // not actually part of ToC
    };

    struct audio_disk_toc {
        u_int8_t start_track;
        u_int8_t end_track;
        std::vector<audio_disk_toc_entry> entries;
    };
}

#endif //STRUCTS_H
