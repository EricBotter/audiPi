#include "AudioDevice.h"

#include <cstdio>
#include <alsa/asoundlib.h>

namespace audipi {
    AudioDevice::AudioDevice() {
        unsigned int channels = 2;
        unsigned int rate = 44100;

        this->pcm_handle = nullptr;

        int error = snd_pcm_open(&this->pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);

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

        if ((error = snd_pcm_hw_params(this->pcm_handle, hw_params)) < 0) {
            printf("Failed to set HW params: %s\n", snd_strerror(error));
            this->pcm_handle = nullptr;
            return;
        }

        if ((error = snd_pcm_hw_params_get_buffer_size(hw_params, &this->buffer_size)) < 0) {
            printf("Failed to get buffer size: %s\n", snd_strerror(error));
            this->pcm_handle = nullptr;
            return;
        }

        snd_pcm_sw_params_malloc(&sw_params);
        snd_pcm_sw_params_current(this->pcm_handle, sw_params);

#if AUDIPI_DEBUG
        printf("Playback setup successful\n");
#endif
    }

    AudioDevice::~AudioDevice() {
        snd_pcm_drop(this->pcm_handle);
        snd_pcm_close(this->pcm_handle);
    }

    bool AudioDevice::is_init() const {
        return this->pcm_handle != nullptr;
    }

    std::expected<long, int> AudioDevice::enqueue_for_playback(const sample_data *buffer, const size_t size) const {
        if (size == 0) {
            return 0;
        }

        const auto written = snd_pcm_writei(this->pcm_handle, buffer, size);
        if (written == -EAGAIN) {
            return 0;
        }
        if (written < 0) {
            if (int recover = snd_pcm_recover(this->pcm_handle, static_cast<int>(written), 0); recover < 0) {
                return std::unexpected(recover);
            }
        }
        return written;
    }

    void AudioDevice::prepare() const {
        snd_pcm_prepare(this->pcm_handle);
    }

    void AudioDevice::pause() const {
        if (snd_pcm_pause(this->pcm_handle, 1)) {
            printf("Error while pausing!");
        }
    }

    void AudioDevice::resume() const {
        snd_pcm_pause(this->pcm_handle, 0);
    }

    void AudioDevice::reset() const {
        snd_pcm_drop(this->pcm_handle);
        snd_pcm_prepare(this->pcm_handle);
    }

    std::expected<unsigned long, long> AudioDevice::get_samples_in_buffer() const {
        snd_pcm_sframes_t pcm_avail_update = snd_pcm_avail_update(this->pcm_handle);
        if (pcm_avail_update < 0) {
            return std::unexpected(pcm_avail_update);
        }

        return this->buffer_size - pcm_avail_update;
    }

    std::string AudioDevice::render_error(const int error_code) {
        return snd_strerror(error_code);
    }
}
