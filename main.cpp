#include <iostream>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/cdrom.h>

#include "CdRom.h"

int main(int argc, char *argv[]) {

    auto cd_rom = audipi::CdRom("/dev/cdrom");

    if (!cd_rom.is_init()) {
        std::cout << "cannot open /dev/cdrom" << std::endl;
        return -1;
    }

    if (auto result = cd_rom.start(); !result) {
        std::cout << "start: unexpected error: " << result.error() << std::endl;
    }

    if (auto result = cd_rom.get_drive_status(); result) {
        std::cout << "Drive status: " << static_cast<int>(result.value()) << std::endl;
    } else {
        std::cout << "get_drive_status: unexpected error: " << result.error() << std::endl;
    }

    if (auto result = cd_rom.stop(); !result) {
        std::cout << "stop: unexpected error: " << result.error() << std::endl;
    }

    if (auto result = cd_rom.close_tray(); !result) {
        std::cout << "close_tray: unexpected errors: " << result.error() << std::endl;
    }

    if (auto result = cd_rom.eject(); !result) {
        // if (result.error() == audipi::eject_error::cannot_eject)
        //     std::cerr << "cannot open tray of disc drive" << std::endl;
        // if (result.error() == audipi::eject_error::device_busy)
        //     std::cerr << "disc drive busy" << std::endl;
        // if (result.error() == audipi::eject_error::undefined)
        //     std::cerr << "unexpected error" << std::endl;
        std::cout << "eject: unexpected error: " << result.error() << std::endl;
    }

    if (auto result = cd_rom.get_drive_status(); result) {
        std::cout << "Drive status: " << static_cast<int>(result.value()) << std::endl;
    } else {
        std::cout << "get_drive_status: unexpected error: " << result.error() << std::endl;
    }

    return 0;
}
