#include "SampleBuffer.h"

namespace audipi {
    SampleBuffer::SampleBuffer() {
        this->buffer = std::vector<u_int8_t>(44100 * 4); // 1 second of 44.1kHz stereo audio
        this->head = 0;
        this->tail = 0;
    }

    size_t SampleBuffer::size() const {
        return (this->buffer.size() + this->head - this->tail) % this->buffer.size();
    }

    void SampleBuffer::push_samples(const u_int8_t *samples, const size_t count) {
        // circular buffer
        size_t size = this->buffer.size() - this->head + this->tail;
        while (size < count) {
            this->reallocate_buffer();
            size = this->buffer.size() - this->head + this->tail;
        }

        for (size_t i = 0; i < count; ++i) { // todo: optimize
            this->buffer[this->head] = samples[i];
            this->head = (this->head + 1) % this->buffer.size();
        }
    }

    void SampleBuffer::pop_samples(u_int8_t *samples, const size_t count) {
        for (size_t i = 0; i < count; ++i) { // todo: optimize
            samples[i] = this->buffer[this->tail];
            this->tail = (this->tail + 1) % this->buffer.size();
        }
    }

    void SampleBuffer::read_samples(u_int8_t *samples, const size_t count, const size_t offset) const {
        for (size_t i = 0; i < count; ++i) { // todo: optimize
            samples[i] = this->buffer[(this->tail + offset + i) % this->buffer.size()];
        }
    }

    void SampleBuffer::discard_samples(const size_t count) {
        this->tail = (this->tail + count) % this->buffer.size();
    }

    void SampleBuffer::reallocate_buffer() {
        std::vector<u_int8_t> new_buffer(this->buffer.size() * 2);
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
}