#include "SampleBuffer.h"

#include <cstdio>

#include "structs.h"

namespace audipi {
    SampleBuffer::SampleBuffer() {
        this->buffer = std::vector<sample_data>(44100); // 1 second of 44.1kHz stereo audio
    }

    size_t SampleBuffer::size() const {
        std::lock_guard lg(this->mutex);
        return (this->buffer.size() + this->head - this->tail) % this->buffer.size();
    }

    void SampleBuffer::push_samples(const sample_data *samples, const size_t count) {
        std::lock_guard lg(this->mutex);

        size_t available_space = this->available_space();
        while (available_space <= count) {
            this->reallocate_buffer();
            available_space = this->available_space();
        }

        for (size_t i = 0; i < count; ++i) { // todo: optimize
            this->buffer[this->head] = samples[i];
            this->head = (this->head + 1) % this->buffer.size();
        }
    }

    void SampleBuffer::pop_samples(sample_data *samples, const size_t count) {
        std::lock_guard lg(this->mutex);

        for (size_t i = 0; i < count; ++i) { // todo: optimize
            samples[i] = this->buffer[this->tail];
            this->tail = (this->tail + 1) % this->buffer.size();
        }
    }

    void SampleBuffer::read_samples(sample_data *samples, const size_t count, const size_t offset) const {
        std::lock_guard lg(this->mutex);

        for (size_t i = 0; i < count; ++i) { // todo: optimize
            samples[i] = this->buffer[(this->tail + offset + i) % this->buffer.size()];
        }
    }

    void SampleBuffer::discard_samples(const size_t count) {
        std::lock_guard lg(this->mutex);

        this->tail = (this->tail + count) % this->buffer.size();
    }

    void SampleBuffer::discard() {
        std::lock_guard lg(this->mutex);

        this->tail = this->head = 0;
    }

    void SampleBuffer::reallocate_buffer() {
#if AUDIPI_DEBUG
        printf("    Reallocating buffer from %lu to %lu\n", this->buffer.size(), this->buffer.size() * 2);
#endif
        std::vector<sample_data> new_buffer(this->buffer.size() * 2);
        size_t i = 0;
        while (this->tail != this->head) {
            new_buffer[i] = this->buffer[this->tail];
            this->tail = (this->tail + 1) % this->buffer.size();
            i++;
        }
        this->tail = 0;
        this->head = i;
        this->buffer = new_buffer;
    }

    std::size_t SampleBuffer::available_space() const {
        ssize_t used = static_cast<ssize_t>(this->head) - static_cast<ssize_t>(this->tail);
        if (used < 0) {
            used += static_cast<ssize_t>(this->buffer.size());
        }
        return this->buffer.size() - used;
    }
}
