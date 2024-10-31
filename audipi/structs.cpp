#include "structs.h"

#include <linux/cdrom.h>

constexpr u_int8_t FRAME_LIMIT = CD_FRAMES;
constexpr u_int8_t SEC_LIMIT = CD_SECS;
constexpr u_int16_t SAMPLE_LIMIT = 588;

namespace audipi {
    msf_location operator+(const msf_location &left, const msf_location &right) {
        u_int8_t frames = left.frame + right.frame;
        u_int8_t secs = left.second + right.second + frames / FRAME_LIMIT;
        u_int8_t mins = left.minute + right.minute + secs / SEC_LIMIT;

        secs %= SEC_LIMIT;
        frames %= FRAME_LIMIT;

        return {mins, secs, frames};
    }

    msf_location operator-(const msf_location &left, const msf_location &right) {
        u_int8_t frames = left.frame - right.frame;
        u_int8_t secs = left.second - right.second;
        u_int8_t mins = left.minute - right.minute;

        if (frames > FRAME_LIMIT) {
            frames += FRAME_LIMIT;
            secs--;
        }

        if (secs > SEC_LIMIT) {
            secs += SEC_LIMIT;
            mins--;
        }

        return {mins, secs, frames};
    }

    msf_location& operator+=(msf_location &left, const msf_location &right) {
        left = left + right;
        return left;
    }

    msf_location& operator-=(msf_location &left, const msf_location &right) {
        left = left - right;
        return left;
    }

    msf_location operator+(const msf_location &left, const size_t &samples) {
        const size_t frames = samples / SAMPLE_LIMIT;
        const msf_location right = {
            static_cast<u_int8_t>(frames / FRAME_LIMIT / SEC_LIMIT),
            static_cast<u_int8_t>(frames / FRAME_LIMIT % SEC_LIMIT),
            static_cast<u_int8_t>(frames % FRAME_LIMIT)
        };
        return left + right;
    }

    msfs_location operator+(const msfs_location &left, const msfs_location &right) {
        u_int16_t samples = left.samples + right.samples;
        u_int8_t frames = left.frame + right.frame + samples / SAMPLE_LIMIT;
        u_int8_t secs = left.second + right.second + frames / FRAME_LIMIT;
        u_int8_t mins = left.minute + right.minute + secs / SEC_LIMIT;

        secs %= SEC_LIMIT;
        frames %= FRAME_LIMIT;
        samples %= SAMPLE_LIMIT;

        return {mins, secs, frames, samples};
    }

    msfs_location operator-(const msfs_location &left, const msfs_location &right) {
        u_int16_t samples = left.samples - right.samples;
        u_int8_t frames = left.frame - right.frame;
        u_int8_t secs = left.second - right.second;
        u_int8_t mins = left.minute - right.minute;

        if (samples > SAMPLE_LIMIT) {
            samples += SAMPLE_LIMIT;
            frames--;
        }

        if (frames > FRAME_LIMIT) {
            frames += FRAME_LIMIT;
            secs--;
        }

        if (secs > SEC_LIMIT) {
            secs += SEC_LIMIT;
            mins--;
        }

        return {mins, secs, frames, samples};
    }

    msfs_location & operator+=(msfs_location &left, const msfs_location &right) {
        left = left + right;
        return left;
    }

    msfs_location & operator-=(msfs_location &left, const msfs_location &right) {
        left = left - right;
        return left;
    }

    msfs_location operator+(const msfs_location &left, const size_t &samples) {
        const msfs_location right = {
            static_cast<u_int8_t>(samples / SAMPLE_LIMIT / FRAME_LIMIT / SEC_LIMIT),
            static_cast<u_int8_t>(samples / SAMPLE_LIMIT / FRAME_LIMIT % SEC_LIMIT),
            static_cast<u_int8_t>(samples / SAMPLE_LIMIT % FRAME_LIMIT),
            static_cast<u_int16_t>(samples % SAMPLE_LIMIT)
        };
        return left + right;
    }

    msfs_location operator-(const msfs_location &left, const size_t &samples) {
        const msfs_location right = {
            static_cast<u_int8_t>(samples / SAMPLE_LIMIT / FRAME_LIMIT / SEC_LIMIT),
            static_cast<u_int8_t>(samples / SAMPLE_LIMIT / FRAME_LIMIT % SEC_LIMIT),
            static_cast<u_int8_t>(samples / SAMPLE_LIMIT % FRAME_LIMIT),
            static_cast<u_int16_t>(samples % SAMPLE_LIMIT)
        };
        return left - right;
    }
}
