#ifndef SAMPLEBUFFER_H
#define SAMPLEBUFFER_H
#include <mutex>
#include <map>
#include <set>

#include "structs.h"

template<> struct std::less<audipi::msf_location>
{
    bool operator() (const audipi::msf_location& lhs, const audipi::msf_location& rhs) const {
        if (lhs.minute == rhs.minute) {
            if (lhs.second == rhs.second) {
                return lhs.frame < rhs.frame;
            }
            return lhs.second < rhs.second;
        }
        return lhs.minute < rhs.minute;
    }
};

namespace audipi {
    class SampleBuffer {
        std::map<msf_location, std::array<sample_data, SAMPLES_IN_FRAME>> cache;

        std::set<msf_location> expired;

        size_t max_samples;

        mutable std::mutex mutex;

    public:
        explicit SampleBuffer(size_t max_samples);

        SampleBuffer(const SampleBuffer& other);

        void add_frame(msf_location location, const std::array<sample_data, SAMPLES_IN_FRAME> &samples);

        bool has_frame(const msf_location &location) const;

        std::array<sample_data, SAMPLES_IN_FRAME> read_frame(const msf_location &location);

        void discard_frame(const msf_location &location);

        void discard();
    };
}

#endif //SAMPLEBUFFER_H
