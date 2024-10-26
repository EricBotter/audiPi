#ifndef AUDIODEVICE_H
#define AUDIODEVICE_H
#include <cstdint>
#include <expected>
#include <string>

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

        [[nodiscard]] std::expected<long, int> enqueue_for_playback_sync(const uint8_t *buffer, std::size_t size) const;

        void prepare() const;

        [[nodiscard]] std::expected<unsigned long, long> get_samples_in_buffer() const;

        [[nodiscard]] static std::string render_error(int error_code);
    };
}

#endif //AUDIODEVICE_H
