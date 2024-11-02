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
        if (this->state == PlayerState::PLAYING) {
            return;
        }

        if (this->state == PlayerState::PAUSED) {
            this->state = PlayerState::PLAYING;
            this->audio_device.resume();
        } else {
            this->state = PlayerState::PLAYING;
            this->current_track = 0;
            this->tracks[current_track].reset();
            this->audio_device.prepare();
        }
    }

    void Player::pause() {
        if (this->state == PlayerState::PLAYING) {
            this->state = PlayerState::PAUSED;
            this->audio_device.pause();
        }
    }

    void Player::stop() {
        if (this->state == PlayerState::PLAYING || this->state == PlayerState::PAUSED) {
            this->tracks[current_track].reset();
        }
        this->state = PlayerState::STOPPED;
        this->current_track = 0;
        this->audio_device.reset();
    }

    void Player::next_track() {
        if (current_track >= tracks.size() - 1)
            return;
        this->sample_buffer.clear();
        this->tracks[current_track++].reset();
    }

    void Player::prev_track() {
        if (current_track <= 0)
            return;
        this->sample_buffer.clear();
        this->tracks[current_track--].reset();
    }

    void Player::tick() {
        if (this->state == PlayerState::ERROR) {
            return;
        }

        constexpr int sufficient_samples = 44100; // 1 second

#if AUDIPI_DEBUG
        printf("Player::tick %ld\n", std::chrono::system_clock::now().time_since_epoch().count());
#endif

        while (sample_buffer.size() < sufficient_samples) {
#if AUDIPI_DEBUG
            printf("  Reading %d samples from CD\n", sufficient_samples);
#endif
            if (auto result = tracks[current_track].pop_samples(sufficient_samples)) {
                sample_buffer.push_samples(result.value().data(), result.value().size());
            } else {
#if AUDIPI_DEBUG
                printf("  Error reading samples from CD\n");
#endif
                this->state = PlayerState::ERROR;
                return;
            }
        }

        if (this->state == PlayerState::PLAYING) {
            const auto samples_in_buffer_maybe = audio_device.get_samples_in_buffer();
            if (!samples_in_buffer_maybe) {
#if AUDIPI_DEBUG
                printf("  Error reading samples in audio device buffer\n");
#endif
                this->state = PlayerState::ERROR;
                return;
            }

            if (const auto samples_in_buffer = samples_in_buffer_maybe.value(); samples_in_buffer > sufficient_samples) {
                return;
            }

#if AUDIPI_DEBUG
            printf("  Enqueuing %d samples for playback\n", sufficient_samples);
            printf("  Audio device buffer size: %ld\n", samples_in_buffer);
#endif

            sample_data samples[sufficient_samples];
            sample_buffer.pop_samples(samples, sufficient_samples);

            if (const auto enqueue_for_playback_maybe = audio_device.enqueue_for_playback(
                reinterpret_cast<u_int8_t *>(samples), sufficient_samples * 4)) {
#if AUDIPI_DEBUG
                printf("  Enqueued %ld samples for playback\n", enqueue_for_playback_maybe.value() / 4);
#endif
            } else {
#if AUDIPI_DEBUG
                printf("  Error enqueuing samples for playback\n");
#endif
                this->state = PlayerState::ERROR;
            }
        }
    }

    Player::player_status Player::get_status() {
        msfs_location current_location = this->tracks[current_track].get_current_location();

        if (const auto samples_in_buffer = audio_device.get_samples_in_buffer()) {
            current_location = current_location - sample_buffer.size() - samples_in_buffer.value();
        } else {
            this->state = PlayerState::ERROR;
            return {
                PlayerState::ERROR,
                0,
                "Error",
                {0, 0, 0}
            };
        }

        return {
            .state = this->state,
            .current_track_index = this->current_track,
            .current_track_name = this->tracks[current_track].get_track_name(),
            .current_location_in_track = current_location
        };
    }
}
