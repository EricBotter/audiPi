#include <iostream>
#include <iomanip>
#include <cstring>
#include <csignal>
#include <sstream>

#include "audipi/CdRom.h"

std::string render_error(const int error_num) {
    std::ostringstream ss;
    ss << strerror(error_num) << " (" << error_num << ")";
    return ss.str();
}

static int keepRunning = 1;

void intHandler(int dummy) {
    keepRunning = 0;
}

int main(int argc, char *argv[]) {
    signal(SIGINT, intHandler);
    signal(SIGTERM, intHandler);

    auto cd_rom = audipi::CdRom("/dev/cdrom");

    if (!cd_rom.is_init()) {
        std::cout << "cannot open /dev/cdrom" << std::endl;
        return -1;
    }

    if (auto drive_status_result = cd_rom.get_drive_status()) {
        if (drive_status_result.value() != audipi::drive_status::disc_ok) {
            std::cout << "Drive status not OK: " << static_cast<int>(drive_status_result.value()) << std::endl
                    << "Exiting..." << std::endl;
            return 0;
        }
    } else {
        std::cout << "get_drive_status: unexpected error: " << render_error(drive_status_result.error()) << std::endl;
    }

    // not required, here for demonstration purposes
    if (auto result = cd_rom.start(); !result) {
        std::cout << "start: unexpected error: " << render_error(result.error()) << std::endl;
    }

    std::vector<audipi::disk_toc_entry> toc_entries{};

    if (auto disk_type = cd_rom.get_disk_type(); disk_type == audipi::disk_type::unsupported) {
        std::cout << "get_disk_type: unsupported disk" << std::endl;
    } else {
        std::cout << "get_disk_type: disk of type " << static_cast<int>(disk_type) << std::endl
                << "Reading TOC..." << std::endl;

        if (auto result = cd_rom.read_toc()) {
            const auto& audio_disk_toc = result.value();

            toc_entries = audio_disk_toc.entries;

            std::cout << "Start track: " << +audio_disk_toc.start_track
                    << " - End track: " << +audio_disk_toc.end_track << std::endl;

            for (auto [track_num, address, duration]: toc_entries) {
                std::cout << "Track " << std::setfill('0') << std::setw(2) << +track_num <<
                    ": Starts at " <<
                    std::setfill('0') << std::setw(2) << +address.minute << ":" <<
                    std::setfill('0') << std::setw(2) << +address.second << "." <<
                    std::setfill('0') << std::setw(2) << +address.frame <<
                    " - Duration: " <<
                    std::setfill('0') << std::setw(2) << +duration.minute << ":" <<
                    std::setfill('0') << std::setw(2) << +duration.second << "." <<
                    std::setfill('0') << std::setw(2) << +duration.frame << std::endl;
            }
        } else {
            std::cout << "read_toc: cannot read TOC: " << render_error(result.error()) << std::endl;
        }
    }

    if (!toc_entries.empty()) {
        std::cout << std::endl << "Start reading CD..." << std::endl;

        audipi::msf_location current_location{0, 2, 0};

        // ReSharper disable once CppDFALoopConditionNotUpdated
        while (keepRunning) {
            auto audio_frame = cd_rom.read_frame(current_location);

            if (!audio_frame) {
                std::cout << "read_frame: unexpected error: " << render_error(audio_frame.error()) << std::endl;
                break;
            }

            std::array<int16_t, 588 * 2> packed_data{};
            std::array<int16_t, 588> left_channel{};
            std::array<int16_t, 588> right_channel{};

            memcpy(packed_data.data(), audio_frame.value().raw_data.data(), 2352);

            for (int i = 0; i < 588; ++i) {
                left_channel[i] = std::abs(packed_data[i * 2]);
                right_channel[i] = std::abs(packed_data[i * 2 + 1]);
            }

            const auto left_max = *std::max_element(left_channel.begin(), left_channel.end());
            const auto right_max = *std::max_element(right_channel.begin(), right_channel.end());

            std::cout << "\rTrack " << std::setfill('0') << std::setw(2) << +audio_frame.value().track_num <<
                " - Index " << +audio_frame.value().index_num <<
                " - [" <<
                std::setfill('0') << std::setw(2) << +audio_frame.value().location_rel.minute << ":" <<
                std::setfill('0') << std::setw(2) << +audio_frame.value().location_rel.second << "." <<
                std::setfill('0') << std::setw(2) << +audio_frame.value().location_rel.frame <<
                "] - [" << std::setfill(' ') << std::setw(33) << std::string(left_max/327/3, '#')
            << "|" <<  std::setw(33) << std::left << std::string(right_max/327/3, '#') << "]" << std::right << std::flush;

            current_location = current_location + audipi::msf_location{0, 0, 1};

            audio_frame = cd_rom.read_frame(current_location);
        }
    }

    std::cout << std::endl << "Stopping CD..." << std::endl;

    if (auto result = cd_rom.stop(); !result) {
        std::cout << "stop: unexpected error: " << render_error(result.error()) << std::endl;
    }

    return 0;

    // Unsupported by my disk drive
    // if (auto result = cd_rom.close_tray(); !result) {
    //     std::cout << "close_tray: unexpected errors: " << render_error(result.error()) << std::endl;
    // }

    if (auto result = cd_rom.eject(); !result) {
        // if (render_error(result.error()) == audipi::eject_error::cannot_eject)
        //     std::cerr << "cannot open tray of disc drive" << std::endl;
        // if (render_error(result.error()) == audipi::eject_error::device_busy)
        //     std::cerr << "disc drive busy" << std::endl;
        // if (render_error(result.error()) == audipi::eject_error::undefined)
        //     std::cerr << "unexpected error" << std::endl;
        std::cout << "eject: unexpected error: " << render_error(result.error()) << std::endl;
    }

    if (auto result = cd_rom.get_drive_status()) {
        std::cout << "Drive status: " << static_cast<int>(result.value()) << std::endl;
    } else {
        std::cout << "get_drive_status: unexpected error: " << render_error(result.error()) << std::endl;
    }

    return 0;
}
