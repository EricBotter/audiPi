#include "SampleBuffer.h"

#include "structs.h"

namespace audipi {
    SampleBuffer::SampleBuffer(const size_t max_samples) {
        this->max_samples = max_samples;
    }

    SampleBuffer::SampleBuffer(const SampleBuffer &other): cache(other.cache), expired(other.expired), max_samples(other.max_samples) {
    }

    void SampleBuffer::add_frame(const msf_location location, const std::array<sample_data, SAMPLES_IN_FRAME> &samples) {
        std::lock_guard lg(this->mutex);

        if (this->cache.size() == max_samples) {
            for (auto expired_i : this->expired) {
                this->cache.erase(expired_i);
            }
            this->expired.clear();
        }

        this->cache[location] = samples;

        if (const auto found = std::ranges::find(this->expired, location); found != this->expired.end()) {
            this->expired.erase(found);
        }
    }

    bool SampleBuffer::has_frame(const msf_location &location) const {
        return this->cache.contains(location);
    }

    std::array<sample_data, SAMPLES_IN_FRAME> SampleBuffer::read_frame(const msf_location &location) {
        std::lock_guard lg(this->mutex);

        this->expired.insert(location);
        return this->cache[location];
    }

    void SampleBuffer::discard_frame(const msf_location &location) {
        std::lock_guard lg(this->mutex);

        this->expired.insert(location);
    }

    void SampleBuffer::discard() {
        std::lock_guard lg(this->mutex);

        cache.clear();
        expired.clear();
    }
}
