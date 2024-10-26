#include "Player.h"

#include <chrono>

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

        printf("Player::tick %ld\n", std::chrono::system_clock::now().time_since_epoch().count());

        while (sample_buffer.size() < sufficient_samples) {
            printf("  Reading %d samples from CD\n", sufficient_samples);
            if (auto result = tracks[current_track].pop_samples(sufficient_samples)) {
                sample_buffer.push_samples(result.value().data(), result.value().size());
            } else {
                printf("  Error reading samples from CD\n");
                this->status = PlayerStatus::ERROR;
                return;
            }
        }

        if (this->status == PlayerStatus::PLAYING) {
            const auto samples_in_buffer_maybe = audio_device.get_samples_in_buffer();
            if (!samples_in_buffer_maybe) {
                printf("  Error reading samples in audio device buffer\n");
                this->status = PlayerStatus::ERROR;
                return;
            }

            const auto samples_in_buffer = samples_in_buffer_maybe.value();

            if (samples_in_buffer > sufficient_samples) {
                return;
            }

            printf("  Enqueuing %d samples for playback\n", sufficient_samples);
            printf("  Audio device buffer size: %ld\n", samples_in_buffer);

            sample_data samples[sufficient_samples];
            sample_buffer.pop_samples(samples, sufficient_samples);

            if (const auto enqueue_for_playback_maybe = audio_device.enqueue_for_playback(
                reinterpret_cast<u_int8_t *>(samples), sufficient_samples * 4)) {
                printf("  Enqueued %ld samples for playback\n", enqueue_for_playback_maybe.value());
            } else {
                printf("  Error enqueuing samples for playback\n");
                this->status = PlayerStatus::ERROR;
            }
        }
    }
}
