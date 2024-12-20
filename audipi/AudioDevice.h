#ifndef AUDIODEVICE_H
#define AUDIODEVICE_H

#include <expected>
#include <string>

#include "structs.h"

// Forward declaration of the ALSA types, expected to be present at link time
using snd_pcm_t = struct _snd_pcm; // NOLINT(*-reserved-identifier)
using snd_timestamp_t = timeval;

namespace audipi {
    class AudioDevice {
        snd_pcm_t *pcm_handle;
        unsigned long buffer_size{};

    public:
        AudioDevice();
        ~AudioDevice();

        [[nodiscard]] bool is_init() const;

        [[nodiscard]] std::expected<long, int> enqueue_for_playback(const sample_data *buffer, std::size_t size) const;

        void prepare() const;

        void pause() const;

        void resume() const;

        void reset() const;

        [[nodiscard]] std::expected<unsigned long, long> get_samples_in_buffer() const;

        [[nodiscard]] static std::string render_error(int error_code);
    };
}

#endif //AUDIODEVICE_H
