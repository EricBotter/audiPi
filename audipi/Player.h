#ifndef PLAYER_H
#define PLAYER_H

#include "AudioDevice.h"
#include "CdRom.h"
#include "PlayerTrack.h"
#include "SampleBuffer.h"
#include "structs.h"

namespace audipi {
    enum class PlayerStatus {
        INIT,
        PLAYING,
        PAUSED,
        STOPPED,
        ERROR
    };

    class Player {
        AudioDevice audio_device;
        SampleBuffer sample_buffer;
        std::vector<CdPlayerTrack> tracks; // todo generalize to PlayerTrack
        size_t current_track{};
        PlayerStatus status = PlayerStatus::INIT;

    public:
        Player();

        [[nodiscard]] bool is_init() const; // todo replace with get player status

        void enqueue_cd(CdRom &cd_rom, const disk_toc &toc);

        void play();

        // void pause();

        void stop();

        void tick();
    };
}

#endif //PLAYER_H
