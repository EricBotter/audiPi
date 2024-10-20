#include <iostream>
#include <iomanip>
#include <cstring>
#include <csignal>
#include <sstream>

#include "audipi/AudioDevice.h"
#include "audipi/CdRom.h"
#include "audipi/SampleBuffer.h"

std::string render_error(const int error_num) {
    std::ostringstream ss;
    ss << strerror(error_num) << " (" << error_num << ")";
    return ss.str();
}

static int keepRunning = 1;

void intHandler(int dummy) {
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
        if (drive_status_result.value() != audipi::drive_status::disc_ok) {
            std::cout << "Drive status not OK: " << static_cast<int>(drive_status_result.value()) << std::endl
                    << "Exiting..." << std::endl;
            return 0;
        }
    } else {
        std::cout << "get_drive_status: unexpected error: " << render_error(drive_status_result.error()) << std::endl;
    }

    std::cout << "Setup audio device..." << std::endl;

    auto audio_device = audipi::AudioDevice();

    if (!audio_device.is_init()) {
        std::cout << "Failed to setup audio device" << std::endl;
        return -1;
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
            const auto &audio_disk_toc = result.value();

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
        audipi::SampleBuffer sample_buffer{};

        std::cout << "Start reading CD..." << std::endl;

        audipi::msf_location current_location{0, 2, 0};

        auto audio_frame = cd_rom.read_frame(current_location);
        audio_device.set_playback_start_position();

        size_t playback_position = 0;

        // ReSharper disable once CppDFALoopConditionNotUpdated
        while (keepRunning) {
            if (!audio_frame) {
                std::cout << "read_frame: unexpected error: " << render_error(audio_frame.error()) << std::endl;
                break;
            }

            auto frame = audio_frame.value();

            sample_buffer.push_samples(frame.raw_data.data(), frame.raw_data.size());

            auto result = audio_device.enqueue_for_playback_sync(frame.raw_data.data(), frame.raw_data.size());
            if (!result) {
                std::cout << "enqueue_for_playback_sync: unexpected error: " <<
                        audipi::AudioDevice::render_error(result.error()) << std::endl;
                break;
            }
            if (result.value() != frame.raw_data.size()) {
                std::cout << "enqueue_for_playback_sync: unexpected number of bytes written: " << result.value() <<
                    " Likely buffer underrun. Resetting playback position..." << std::endl;

                audio_device.set_playback_start_position();
                result = audio_device.enqueue_for_playback_sync(frame.raw_data.data(), frame.raw_data.size());
                if (!result) {
                    std::cout << "enqueue_for_playback_sync: unexpected error after playback reset: " <<
                            audipi::AudioDevice::render_error(result.error()) << std::endl;
                    std::cout << "Exiting..." << std::endl;
                    break;
                }
            }

            auto new_playback_position = audio_device.get_playback_position();
            std::cout << "Playback position: " << new_playback_position << "\t\t\t";

            sample_buffer.discard_samples((new_playback_position - playback_position) * 4);
            print_frame(sample_buffer, audipi::msf_location{0, 2, 0} + playback_position, frame.track_num);
            playback_position = new_playback_position;

            current_location = current_location + audipi::msf_location{0, 0, 1};
            audio_frame = cd_rom.read_frame(current_location);
        }
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

    sample_buffer.read_samples(reinterpret_cast<uint8_t *>(packed_data.data()), 2352, 0);

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
