#ifndef PLAYERTRACK_H
#define PLAYERTRACK_H
#include <expected>
#include <vector>

#include "CdRom.h"
#include "SampleBuffer.h"
#include "structs.h"

namespace audipi {
    class CdPlayerTrack {
        CdRom &cd_rom;
        SampleBuffer buffer;
        const disk_toc_entry track;
        msfs_location current_location;

    public:
        CdPlayerTrack(CdRom &cd_rom, const disk_toc_entry &track);

        CdPlayerTrack(const CdPlayerTrack &other);

        void reset();

        [[nodiscard]] std::string get_track_name() const; // todo add tags (artist, album...)

        [[nodiscard]] std::expected<std::vector<sample_data>, int> pop_samples(size_t num_samples);

        void prefetch_samples(size_t num_samples);

        [[nodiscard]] msfs_location get_current_location() const {
            return current_location;
        }
    };
}

#endif //PLAYERTRACK_H
