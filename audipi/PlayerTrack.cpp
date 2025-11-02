#include "PlayerTrack.h"

#if AUDIPI_DEBUG
#include <cstdio>
#endif

#include "util.h"

namespace audipi {
    CdPlayerTrack::CdPlayerTrack(CdRom &cd_rom, const disk_toc_entry &track)
        : cd_rom(cd_rom), buffer(30*75), track(track), current_location{0, 0, 0, 0} {
    }

    CdPlayerTrack::CdPlayerTrack(const CdPlayerTrack &other) = default;

    void CdPlayerTrack::reset() {
        this->current_location = {0, 0, 0, 0}; }

    std::string CdPlayerTrack::get_track_name() const {
        return std::string("CD Track ") + left_pad_string(std::to_string(this->track.track_num), 2, '0');
    }

    template<size_t array_size, typename = std::enable_if_t<array_size % 4 == 0>>
    std::array<sample_data, array_size / 4> copy_from(const std::array<u_int8_t, array_size> &data) {
        std::array<sample_data, array_size / 4> samples{};

        for (size_t i = 0; i < array_size; i += 4) {
            samples[i / 4] = sample_data{data[i], data[i + 1], data[i + 2], data[i + 3]};
        }

        return samples;
    }

    std::expected<std::vector<sample_data>, int> CdPlayerTrack::pop_samples(const size_t num_samples) {
        std::vector<sample_data> samples;

        size_t samples_to_fetch = num_samples;

        while (samples_to_fetch > 0) {
#if AUDIPI_DEBUG
            printf("%lu samples left to fetch\n", samples_to_fetch);
#endif

            const auto num_to_fetch = samples_to_fetch > SAMPLES_IN_FRAME - current_location.samples
                    ? SAMPLES_IN_FRAME - current_location.samples
                    : samples_to_fetch;

#if AUDIPI_DEBUG
            printf("  Fetching %lu samples\n", num_to_fetch);
            printf("  From location %s\n", msfs_location_to_string(current_location).c_str());
#endif

            if (buffer.has_frame(current_location)) {

#if AUDIPI_DEBUG
                printf("  Cache hit!\n");
#endif

                const auto frame_data = buffer.read_frame(current_location);

                samples.insert(samples.end(),
                    std::make_move_iterator(frame_data.begin() + current_location.samples),
                    std::make_move_iterator(frame_data.begin() + current_location.samples + num_to_fetch));

            } else {

#if AUDIPI_DEBUG
                printf("  Cache miss, reading from CD\n");
#endif

                if (auto result = cd_rom.read_frame(current_location + track.address)) {
                    auto frame_samples = copy_from(result.value().raw_data);
                    buffer.add_frame(current_location, frame_samples);

                    samples.insert(samples.end(),
                        std::make_move_iterator(frame_samples.begin() + current_location.samples),
                        std::make_move_iterator(frame_samples.begin() + current_location.samples + num_to_fetch));
                } else {
                    return std::unexpected(result.error());
                }
            }

            current_location += num_to_fetch;
            samples_to_fetch -= num_to_fetch;
        }

        return samples;
    }

    void CdPlayerTrack::prefetch_samples(const size_t num_samples) {
        size_t samples_to_fetch = num_samples;
        msf_location location = current_location;
#if AUDIPI_DEBUG
        printf("Prefetching %ld samples\n", samples_to_fetch);
#endif

        while (samples_to_fetch > 0) {
            if (!buffer.has_frame(location)) {
                if (auto result = cd_rom.read_frame(location + track.address)) {
                    auto frame_samples = copy_from(result.value().raw_data);
                    buffer.add_frame(location, frame_samples);

#if AUDIPI_DEBUG
                    printf("  Added frame %d:%d:%d to cache\n", location.minute, location.second, location.frame);
#endif
                } else {
                    break;
                }
            }

            samples_to_fetch -= SAMPLES_IN_FRAME;
            location = location + SAMPLES_IN_FRAME;
        }
    }
} // namespace audipi
