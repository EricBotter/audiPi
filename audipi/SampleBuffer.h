#ifndef SAMPLEBUFFER_H
#define SAMPLEBUFFER_H
#include <vector>
#include <sys/types.h>

namespace audipi {
    class SampleBuffer {
        std::vector<u_int8_t> buffer;
        size_t head;
        size_t tail;
        void reallocate_buffer();

    public:
        SampleBuffer();

        [[nodiscard]] size_t size() const;

        void push_samples(const u_int8_t *samples, size_t count);

        void pop_samples(u_int8_t *samples, size_t count);

        void read_samples(u_int8_t *samples, size_t count, size_t offset) const;

        void discard_samples(size_t count);
    };
}

#endif //SAMPLEBUFFER_H
