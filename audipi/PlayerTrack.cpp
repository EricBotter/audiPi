#include "PlayerTrack.h"

namespace audipi {
    CdPlayerTrack::CdPlayerTrack(CdRom &cd_rom, const disk_toc_entry &track)
        : cd_rom(cd_rom), track(track) {
    }

    void CdPlayerTrack::reset() {
        this->current_location = {
            .location = {0, 0, 0},
            .samples = 0
        };
    }

    template<size_t array_size>
    std::vector<sample_data> copy_from(const std::array<u_int8_t, array_size> &data) {
        std::vector<sample_data> samples;
        samples.reserve(array_size / 4);

        for (size_t i = 0; i < array_size; i += 4) {
            samples.push_back({
                data[i], data[i + 1], data[i + 2], data[i + 3]
            });
        }

        return samples;
    }

    std::expected<std::vector<sample_data>, int> CdPlayerTrack::pop_samples(size_t num_samples) {
        std::vector<sample_data> samples;

        if (current_location.samples != 0) {
            if (auto result = cd_rom.read_frame(current_location.location + track.address)) {
                auto frame_samples = copy_from(result.value().raw_data);

                samples.insert(samples.end(),
                               std::make_move_iterator(
                                   frame_samples.begin() + static_cast<std::vector<sample_data>::difference_type>(
                                       current_location.samples)),
                               std::make_move_iterator(frame_samples.end()));
            } else {
                return std::unexpected(result.error());
            }

            current_location.samples = 0;
            current_location.location += msf_location{0, 0, 1};
        }

        for (auto i = samples.size(); i < num_samples; i += 588) {
            if (auto result = cd_rom.read_frame(current_location.location + track.address)) {
                auto frame_samples = copy_from(result.value().raw_data);

                samples.insert(samples.end(), std::make_move_iterator(frame_samples.begin()),
                               std::make_move_iterator(frame_samples.end()));
                current_location.location += msf_location{0, 0, 1};
            } else {
                return std::unexpected(result.error());
            }
        }

        if (samples.size() > num_samples) {
            current_location.location -= msf_location{0, 0, 1};
            current_location.samples = 588 - (samples.size() - num_samples);
            samples.resize(num_samples);
        }

        return std::move(samples);
    }
}
