#ifndef SAMPLEBUFFER_H
#define SAMPLEBUFFER_H
#include <vector>

#include "structs.h"

namespace audipi {
    class SampleBuffer {
        std::vector<sample_data> buffer;
        size_t head;
        size_t tail;
        void reallocate_buffer();
        [[nodiscard]] std::size_t available_space() const;

    public:
        SampleBuffer();

        [[nodiscard]] size_t size() const;

        void push_samples(const sample_data *samples, size_t count);

        void pop_samples(sample_data *samples, size_t count);

        void read_samples(sample_data *samples, size_t count, size_t offset) const;

        void discard_samples(size_t count);

        void discard();
    };
}

#endif //SAMPLEBUFFER_H
