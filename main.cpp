#include <iostream>
#include <iomanip>
#include <cstring>
#include <sstream>

#include "audipi/CdRom.h"

std::string render_error(const int error_num) {
    std::ostringstream ss;
    ss << strerror(error_num) << " (" << error_num << ")";
    return ss.str();
}

int main(int argc, char *argv[]) {
    auto cd_rom = audipi::CdRom("/dev/cdrom");

    if (!cd_rom.is_init()) {
        std::cout << "cannot open /dev/cdrom" << std::endl;
        return -1;
    }

    if (auto drive_status_result = cd_rom.get_drive_status()) {
        std::cout << "Drive status: " << static_cast<int>(drive_status_result.value()) << std::endl;

        if (drive_status_result.value() == audipi::drive_status::disc_ok) {
            if (auto result = cd_rom.start(); !result) {
                std::cout << "start: unexpected error: " << render_error(result.error()) << std::endl;
            }

            if (auto disk_type = cd_rom.get_disk_type(); disk_type == audipi::disk_type::unsupported) {
                std::cout << "get_disk_type: unsupported disk" << std::endl;
            } else {
                std::cout << "get_disk_type: disk of type " << static_cast<int>(disk_type) << std::endl
                        << "Reading TOC..." << std::endl;

                if (auto result = cd_rom.read_toc()) {
                    auto audio_disk_toc = result.value();
                    std::cout << "Start track: " << +audio_disk_toc.start_track
                            << " - End track: " << +audio_disk_toc.end_track << std::endl;

                    for (auto toc_entry: audio_disk_toc.entries) {
                        std::cout << "Track " << std::setfill('0') << std::setw(2) << +toc_entry.track_num <<
                            ": Starts at " <<
                            std::setfill('0') << std::setw(2) << +toc_entry.address.minute << ":" <<
                            std::setfill('0') << std::setw(2) << +toc_entry.address.second << "." <<
                            std::setfill('0') << std::setw(2) << +toc_entry.address.frame << std::endl;
                    }
                } else {
                    std::cout << "read_toc: cannot read TOC: " << render_error(result.error()) << std::endl;
                }
            }
        }
    } else {
        std::cout << "get_drive_status: unexpected error: " << render_error(drive_status_result.error()) << std::endl;
    }

    if (auto result = cd_rom.stop(); !result) {
        std::cout << "stop: unexpected error: " << render_error(result.error()) << std::endl;
    }

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
