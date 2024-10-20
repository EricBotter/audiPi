#include "Player.h"

namespace audipi {
    Player::Player() = default;

    bool Player::is_init() const {
        return this->audio_device.is_init();
    }

    void Player::enqueue_cd(CdRom &cd_rom, const disk_toc &toc) {
        for (const auto &track: toc.entries) {
            this->tracks.emplace_back(cd_rom, track);
        }
    }

    void Player::play() {
        this->status = PlayerStatus::PLAYING;
        this->current_track = 0;
        this->tracks[current_track].reset();
        this->audio_device.prepare();
    }

    void Player::tick() {
        if (this->status == PlayerStatus::ERROR) {
            return;
        }

        constexpr int sufficient_samples = 44100; // 1 second

        while (sample_buffer.size() < sufficient_samples) {
            if (auto result = tracks[current_track].pop_samples(sufficient_samples)) {
                sample_buffer.push_samples(result.value().data(), result.value().size());
            } else {
                this->status = PlayerStatus::ERROR;
                return;
            }
        }

        if (this -> status == PlayerStatus::PLAYING) {
            sample_data samples[sufficient_samples];

            sample_buffer.pop_samples(samples, sufficient_samples);

            if (audio_device.enqueue_for_playback_sync(reinterpret_cast<u_int8_t*>(samples), sufficient_samples * 4)) {

            } else {
                this->status = PlayerStatus::ERROR;
                return;
            }
        }
    }


}
