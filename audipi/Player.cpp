#include "Player.h"

#include <chrono>

#if AUDIPI_DEBUG
#include "util.h"
#endif

const audipi::Player::player_status ERROR_PLAYER_STATUS = {
    audipi::PlayerState::ERROR,
    0,
    "Error",
    {}
};

namespace audipi {
    void Player::set_error(const char *error) {
        error_cause = error;
        this->state = PlayerState::ERROR;
    }

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
        this->sample_buffer.discard();
        this->tracks[current_track++].reset();
        this->tracks[current_track].reset();
    }

    void Player::prev_track() {
        if (current_track <= 0)
            return;
        this->sample_buffer.discard();
        this->tracks[current_track--].reset();
        this->tracks[current_track].reset();
    }

    void Player::clear_playlist() {
        this->stop();
        this->tracks.clear();
        this->current_track = 0;
    }

    void Player::tick() {
        if (this->state == PlayerState::ERROR || this->state == PlayerState::STOPPED) {
            return;
        }

        constexpr int audio_sufficient_samples = 44100; // 0.1 seconds
        constexpr int sufficient_samples = 44100; // 1 second

        constexpr int internal_low_threshold_samples = 44100 * 15;
        constexpr int internal_high_threshold_samples = 44100 * 45;

#if AUDIPI_DEBUG
        printf("Player::tick %ld\n", std::chrono::system_clock::now().time_since_epoch().count());
        printf("  Filling buffer: %d\n", filling_buffer);
        printf("  Buffer size: %lu\n", sample_buffer.size());
#endif

        if (filling_buffer || sample_buffer.size() < internal_low_threshold_samples) {
            filling_buffer = true;
            if (sample_buffer.size() < internal_high_threshold_samples) {
#if AUDIPI_DEBUG
                printf("  Reading %d samples from CD at %s\n", sufficient_samples, msfs_location_to_string(this
                    ->tracks[current_track].get_current_location()).c_str());
#endif
                if (auto result = tracks[current_track].pop_samples(sufficient_samples)) {
                    sample_buffer.push_samples(result.value().data(), result.value().size());
                } else {
#if AUDIPI_DEBUG
                    printf("  Error reading samples from CD\n");
#endif
                    this->set_error("Error reading samples from CD");
                    return;
                }
            } else {
                filling_buffer = false;
            }
        }

        if (this->state == PlayerState::PLAYING) {
            const auto samples_in_buffer_maybe = audio_device.get_samples_in_buffer();
            if (!samples_in_buffer_maybe) {
#if AUDIPI_DEBUG
                printf("  Error reading samples in audio device buffer: %ld\n", samples_in_buffer_maybe.error());
#endif
                this->set_error("Error reading sample count in audio device buffer in tick");
                return;
            }
#if AUDIPI_DEBUG
            printf("  Audio device buffer size: %ld\n", samples_in_buffer_maybe.value());
#endif

            if (const auto samples_in_buffer = samples_in_buffer_maybe.value(); samples_in_buffer > audio_sufficient_samples) {
                return;
            }
#if AUDIPI_DEBUG
            printf("  Enqueuing %d samples for playback\n", sufficient_samples);
#endif

            sample_data samples[audio_sufficient_samples];
            sample_buffer.pop_samples(samples, audio_sufficient_samples);

            if (const auto enqueue_for_playback_maybe = audio_device.enqueue_for_playback(
                reinterpret_cast<u_int8_t *>(samples), audio_sufficient_samples * 4)) { // todo change with bitcast
#if AUDIPI_DEBUG
                printf("  Enqueued %ld samples for playback\n", enqueue_for_playback_maybe.value() / 4);
#endif
            } else {
#if AUDIPI_DEBUG
                printf("  Error enqueuing samples for playback\n");
#endif
                this->set_error("Error enqueuing samples for playback");
            }
        }
    }

    const std::string & Player::get_error_cause() const {
        return this->error_cause;
    }

    PlayerState Player::get_state() const {
        return state;
    }

    Player::player_status Player::get_status() {

        if (this->state == PlayerState::ERROR) {
            return ERROR_PLAYER_STATUS;
        }

        msfs_location current_location = this->tracks[current_track].get_current_location();

        if (const auto samples_in_buffer = audio_device.get_samples_in_buffer()) {
            current_location = current_location - sample_buffer.size() - samples_in_buffer.value();
        } else {
            this->set_error("Error reading sample count in audio device buffer in get_status");
            return ERROR_PLAYER_STATUS;
        }

        return {
            .state = this->state,
            .current_track_index = this->current_track,
            .current_track_name = this->tracks[current_track].get_track_name(),
            .current_location_in_track = current_location
        };
    }

    std::expected<void, std::string> Player::jump_to_track(const int track_idx) {
        if (track_idx < 0 || track_idx >= this->tracks.size()) {
            return std::unexpected("Out of bounds");
        }
        this->tracks[current_track].reset();
        this->current_track = track_idx;
        this->tracks[current_track].reset();
        return {};
    }
}
