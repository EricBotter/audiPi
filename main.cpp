#include <iostream>
#include <iomanip>
#include <cstring>
#include <csignal>
#include <sstream>
#include <thread>
#include <chrono>
#include <algorithm>

#include "audipi/AudioDevice.h"
#include "audipi/CdRom.h"
#include "audipi/Player.h"
#include "audipi/SampleBuffer.h"

std::string render_error(const int error_num) {
    std::ostringstream ss;
    ss << strerror(error_num) << " (" << error_num << ")";
    return ss.str();
}

static int keepRunning = 1;

void intHandler(int ignored) {
    keepRunning = 0;
}

snd_pcm_t *setup_audio();

void print_frame(const audipi::SampleBuffer &sample_buffer, const audipi::msf_location &current_location,
                 size_t track_num);

int main(int argc, char *argv[]) {
    signal(SIGINT, intHandler);
    signal(SIGTERM, intHandler);

    auto cd_rom = audipi::CdRom("/dev/cdrom");

    if (!cd_rom.is_init()) {
        std::cout << "cannot open /dev/cdrom" << std::endl;
        return -1;
    }

    if (auto drive_status_result = cd_rom.get_drive_status()) {
        if (drive_status_result.value() == audipi::drive_status::no_disk) {
            std::cout << "No disc in disc drive!" << std::endl
                    << "Exiting..." << std::endl;
            return 0;
        }
        if (drive_status_result.value() != audipi::drive_status::disc_ok) {
            std::cout << "Drive status not OK: " << static_cast<int>(drive_status_result.value()) << std::endl
                    << "Exiting..." << std::endl;
            return 0;
        }
    } else {
        std::cout << "get_drive_status: unexpected error: " << render_error(drive_status_result.error()) << std::endl;
    }

    std::cout << "Setup Player..." << std::endl;

    auto player = audipi::Player();

    if (!player.is_init()) {
        std::cout << "cannot start player (error in audio_device init)" << std::endl;
        return -1;
    }

    // not required, here for demonstration purposes
    if (auto result = cd_rom.start(); !result) {
        std::cout << "start: unexpected error: " << render_error(result.error()) << std::endl;
    }

    if (auto disk_type = cd_rom.get_disk_type(); disk_type == audipi::disk_type::unsupported) {
        std::cout << "get_disk_type: unsupported disk" << std::endl;
        return -1;
    }

    std::cout << "Reading TOC..." << std::endl;

    audipi::disk_toc audio_disk_toc;

    if (auto result = cd_rom.read_toc()) {
        audio_disk_toc = result.value();

        std::cout << "Start track: " << +audio_disk_toc.start_track
                << " - End track: " << +audio_disk_toc.end_track << std::endl;

        for (auto [track_num, address, duration]: audio_disk_toc.entries) {
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
        return -1;
    }

    if (!audio_disk_toc.entries.empty()) {
        std::cout << "Enqueuing CD..." << std::endl;

        player.enqueue_cd(cd_rom, audio_disk_toc);
    }

    player.play();

    // ReSharper disable once CppDFALoopConditionNotUpdated
    while (keepRunning) {
        player.tick();

        auto player_status = player.get_status();

        std::cout << "\rPlayer status: " << static_cast<int>(player_status.state)
                << " - Track[" << std::setfill('0') << std::setw(2) << player_status.current_track_index
                << "]: " << player_status.current_track_name
                << " - Location: " << std::setfill('0') << std::setw(2) << +player_status.current_location_in_track.
                minute << ":"
                << std::setfill('0') << std::setw(2) << +player_status.current_location_in_track.second << "."
                << std::setfill('0') << std::setw(2) << +player_status.current_location_in_track.frame <<std::flush;

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << std::endl;
    // std::cout << "Stopping CD..." << std::endl;
    //
    // if (auto result = cd_rom.stop(); !result) {
    //     std::cout << "stop: unexpected error: " << render_error(result.error()) << std::endl;
    // }

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

void print_frame(const audipi::SampleBuffer &sample_buffer, const audipi::msf_location &current_location,
                 const size_t track_num) {
    std::array<int16_t, 588 * 2> packed_data{};
    std::array<int16_t, 588> left_channel{};
    std::array<int16_t, 588> right_channel{};

    // sample_buffer.read_samples(reinterpret_cast<uint8_t *>(packed_data.data()), 2352, 0);

    for (int i = 0; i < 588; ++i) {
        left_channel[i] = static_cast<int16_t>(std::abs(packed_data[i * 2]));
        right_channel[i] = static_cast<int16_t>(std::abs(packed_data[i * 2 + 1]));
    }

    const auto left_max = *std::max_element(left_channel.begin(), left_channel.end());
    const auto right_max = *std::max_element(right_channel.begin(), right_channel.end());

    std::cout << "Track " << std::setfill('0') << std::setw(2) << +track_num <<
            " - [" <<
            std::setfill('0') << std::setw(2) << +current_location.minute << ":" <<
            std::setfill('0') << std::setw(2) << +current_location.second << "." <<
            std::setfill('0') << std::setw(2) << +current_location.frame <<
            "] - [" << std::setfill(' ') << std::setw(33) << std::string(left_max / 327 / 3, '#')
            << "|" << std::setw(33) << std::left << std::string(right_max / 327 / 3, '#') << "]" << std::right
            << "\r" << std::flush;
}
