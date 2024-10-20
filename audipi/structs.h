#ifndef STRUCTS_H
#define STRUCTS_H
#include <vector>
#include <array>
#include <sys/types.h>

namespace audipi {
    struct msf_location {
        u_int8_t minute;
        u_int8_t second;
        u_int8_t frame;
    };

    msf_location operator-(const msf_location& left, const msf_location& right);
    msf_location operator+(const msf_location& left, const msf_location& right);

    msf_location& operator+=(msf_location& left, const msf_location& right);
    msf_location& operator-=(msf_location& left, const msf_location& right);

    msf_location operator+(const msf_location& left, const size_t& samples);

    struct disk_toc_entry {
        u_int8_t track_num;
        msf_location address;
        msf_location duration; // not actually part of ToC
    };

    struct disk_toc {
        u_int8_t start_track;
        u_int8_t end_track;
        std::vector<disk_toc_entry> entries;
    };

    struct audio_frame {
        u_int8_t track_num;
        u_int8_t index_num;
        msf_location location_abs;
        msf_location location_rel;
        std::array<u_int8_t, 2352> raw_data;
    };

    struct sample_data {
        u_int8_t data[4];
    };
}

#endif //STRUCTS_H
