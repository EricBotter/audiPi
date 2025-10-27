#ifndef PLAYER_H
#define PLAYER_H

#include <atomic>
#include <thread>

#include "AudioDevice.h"
#include "CdRom.h"
#include "PlayerTrack.h"
#include "SampleBuffer.h"
#include "structs.h"

namespace audipi {
    enum class PlayerState {
        PLAYING,
        PAUSED,
        STOPPED,
        ERROR
    };

    class Player {
        AudioDevice audio_device;
        std::vector<CdPlayerTrack> tracks; // todo generalize to PlayerTrack
        size_t current_track{};
        PlayerState state = PlayerState::STOPPED;
        std::string error_cause;

        void set_error(const char* error);

    public:
        struct player_status {
            PlayerState state;
            size_t current_track_index;
            std::string current_track_name; // todo implement tags (artist, album...)
            msf_location current_location_in_track;
        };

        Player();

        [[nodiscard]] bool is_init() const; // todo replace with get player status

        void enqueue_cd(CdRom &cd_rom, const disk_toc &toc);

        void play();

        void pause();

        void stop();

        void next_track();

        void prev_track();

        std::expected<void, std::string> jump_to_track(size_t track_idx);

        void clear_playlist();

        void tick();

        [[nodiscard]] const std::string& get_error_cause() const;

        [[nodiscard]] PlayerState get_state() const;

        player_status get_status();
    };
}

#endif //PLAYER_H
