#ifndef STRUCTS_H
#define STRUCTS_H
#include <vector>
#include <array>
#include <sys/types.h>

constexpr auto SAMPLES_IN_FRAME = 588;

namespace audipi {
    /**
    * @brief Represents a location on a CD in MSF format (minute, second, frame)
    */
    struct msf_location {
        u_int8_t minute;
        u_int8_t second;
        u_int8_t frame;
    };

    /**
    * @brief Represents a more accurate location on a CD in MSF format (minute, second, frame) with samples in a frame.
    * There are 588 samples in a frame.
    */
    struct msfs_location {
        u_int8_t minute;
        u_int8_t second;
        u_int8_t frame;
        u_int16_t samples;

        operator msf_location() const;
    };

    msf_location operator-(const msf_location& left, const msf_location& right);
    msf_location operator+(const msf_location& left, const msf_location& right);

    msf_location& operator+=(msf_location& left, const msf_location& right);
    msf_location& operator-=(msf_location& left, const msf_location& right);

    msf_location operator+(const msf_location& left, const size_t& samples);

    msfs_location operator-(const msfs_location& left, const msfs_location& right);
    msfs_location operator+(const msfs_location& left, const msfs_location& right);

    msfs_location& operator+=(msfs_location& left, const msfs_location& right);
    msfs_location& operator-=(msfs_location& left, const msfs_location& right);

    msfs_location operator+(const msfs_location& left, const size_t& samples);
    msfs_location operator-(const msfs_location& left, const size_t& samples);

    msfs_location& operator+=(msfs_location& left, const size_t& samples);

    bool operator==(const msf_location& left, const msf_location& right);

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

    struct cd_audio_frame {
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
