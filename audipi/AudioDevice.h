#ifndef AUDIODEVICE_H
#define AUDIODEVICE_H
#include <cstdint>
#include <expected>
#include <string>

// Forward declaration of the ALSA PCM type, expected to be present at link time
using snd_pcm_t = struct _snd_pcm; // NOLINT(*-reserved-identifier)

namespace audipi {
    class AudioDevice {
        snd_pcm_t *pcm_handle;

    public:
        AudioDevice();
        ~AudioDevice();

        [[nodiscard]] bool is_init() const;

        [[nodiscard]] std::expected<long, int> enqueue_for_playback_sync(const uint8_t *buffer, std::size_t size) const;

        [[nodiscard]] static std::string render_error(int error_code);
    };
}

#endif //AUDIODEVICE_H
