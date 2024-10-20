#ifndef PLAYERTRACK_H
#define PLAYERTRACK_H
#include <expected>
#include <vector>

#include "CdRom.h"
#include "structs.h"

namespace audipi {
    class CdPlayerTrack {
        CdRom &cd_rom;
        const disk_toc_entry &track;

        struct current_location {
            msf_location location;
            size_t samples;
        } current_location;

    public:
        CdPlayerTrack(CdRom &cd_rom, const disk_toc_entry &track);

        void reset();

        [[nodiscard]] std::expected<std::vector<sample_data>, int> pop_samples(size_t num_samples);
    };
}

#endif //PLAYERTRACK_H
