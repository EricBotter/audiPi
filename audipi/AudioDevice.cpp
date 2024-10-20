#include "AudioDevice.h"

#include <cstdio>
#include <alsa/asoundlib.h>

namespace audipi {
    AudioDevice::AudioDevice() {
        this->playback_start_position = {0, 0};

        unsigned int channels = 2;
        unsigned int rate = 44100;
        unsigned long period_size = 2352;

        this->pcm_handle = nullptr;

        int error = snd_pcm_open(&this->pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0);

        if (error < 0) {
            printf("Failed to open PCM device %s\n", snd_strerror(error));
            this->pcm_handle = nullptr;
        }

        snd_pcm_sw_params_t *sw_params = nullptr;
        snd_pcm_hw_params_t *hw_params = nullptr;

        snd_pcm_hw_params_alloca(&hw_params);

        if (snd_pcm_hw_params_any(this->pcm_handle, hw_params) < 0) {
            printf("Failed to retrieve HW params\n");
            this->pcm_handle = nullptr;
            return;
        }

        if ((error = snd_pcm_hw_params_set_access(this->pcm_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
            printf("ERROR: Can't set interleaved mode. %s\n", snd_strerror(error));
            this->pcm_handle = nullptr;
            return;
        }

        if ((error = snd_pcm_hw_params_set_format(this->pcm_handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
            printf("ERROR: Can't set format. %s\n", snd_strerror(error));
            this->pcm_handle = nullptr;
            return;
        }

        if ((error = snd_pcm_hw_params_set_channels(this->pcm_handle, hw_params, channels)) < 0) {
            printf("ERROR: Can't set channels number. %s\n", snd_strerror(error));
            this->pcm_handle = nullptr;
            return;
        }

        if ((error = snd_pcm_hw_params_set_rate_near(this->pcm_handle, hw_params, &rate, nullptr)) < 0) {
            printf("ERROR: Can't set rate. %s\n", snd_strerror(error));
            this->pcm_handle = nullptr;
            return;
        }

        if ((error = snd_pcm_hw_params_set_period_size_near(this->pcm_handle, hw_params, &period_size, nullptr)) < 0) {
            printf("Error: Can't set period size. %s\n", snd_strerror(error));
            this->pcm_handle = nullptr;
            return;
        }

        if ((error = snd_pcm_hw_params(this->pcm_handle, hw_params)) < 0) {
            printf("Failed to set HW params: %s\n", snd_strerror(error));
            this->pcm_handle = nullptr;
            return;
        }

        snd_pcm_sw_params_malloc(&sw_params);
        snd_pcm_sw_params_current(this->pcm_handle, sw_params);

        // snd_pcm_sw_params_set_tstamp_mode(this->pcm_handle, sw_params, SND_PCM_TSTAMP_ENABLE);
        // snd_pcm_sw_params_set_tstamp_type(this->pcm_handle, sw_params, SND_PCM_TSTAMP_TYPE_MONOTONIC);
        //
        // if ((error = snd_pcm_sw_params(this->pcm_handle, sw_params)) < 0) {
        //     printf("Failed to set SW params: %s\n", snd_strerror(error));
        //     this->pcm_handle = nullptr;
        //     return;
        // }

        printf("Playback setup successful\n");
    }

    AudioDevice::~AudioDevice() {
        snd_pcm_drop(this->pcm_handle);
        snd_pcm_close(this->pcm_handle);
    }

    bool AudioDevice::is_init() const {
        return this->pcm_handle != nullptr;
    }

    std::expected<long, int> AudioDevice::enqueue_for_playback_sync(const uint8_t *buffer, const std::size_t size) const {
        if (size % 4) {
            return std::unexpected(0);
        }

        auto written = snd_pcm_writei(this->pcm_handle, buffer, size/4);
        if (written < 0) {
            if (int recover = snd_pcm_recover(this->pcm_handle, static_cast<int>(written), 0); recover < 0) {
                return std::unexpected(recover);
            }
        }
        return written * 4;
    }

    void AudioDevice::set_playback_start_position() {
        snd_pcm_status_t *status;
        snd_pcm_status_malloc(&status);

        snd_pcm_status(pcm_handle, status);
        snd_pcm_status_get_tstamp(status, &this->playback_start_position);

        snd_pcm_status_free(status);
    }

    long AudioDevice::get_playback_position() const {
        snd_pcm_status_t *status;
        snd_pcm_status_malloc(&status);

        snd_pcm_status(pcm_handle, status);
        snd_timestamp_t current_timestamp;
        snd_pcm_status_get_tstamp(status, &current_timestamp);

        long playback_position = (current_timestamp.tv_sec - this->playback_start_position.tv_sec)*44100;
        playback_position += (current_timestamp.tv_usec - this->playback_start_position.tv_usec)*44100/1000000L;

        snd_pcm_status_free(status);

        return playback_position;
    }

    std::string AudioDevice::render_error(const int error_code) {
        return snd_strerror(error_code);
    }
}
