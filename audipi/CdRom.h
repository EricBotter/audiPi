#ifndef CDROM_H
#define CDROM_H
#include <expected>
#include <string>
#include <linux/cdrom.h>

#include "enums.h"
#include "structs.h"

namespace audipi {

    class CdRom {
        int cdrom_fd;

    public:
        explicit CdRom(const std::string &fd_path);

        [[nodiscard]] bool is_init() const;

        [[nodiscard]] std::expected<void, int> start() const;
        [[nodiscard]] std::expected<void, int> stop() const;

        [[nodiscard]] std::expected<void, int> eject() const;
        [[nodiscard]] std::expected<void, int> close_tray() const;

        [[nodiscard]] std::expected<drive_status, int> get_drive_status() const;
        [[nodiscard]] disk_type get_disk_type() const;

        [[nodiscard]] std::expected<disk_toc, int> read_toc() const;

        [[nodiscard]] std::expected<cd_audio_frame, int> read_frame(const msf_location& location) const;

        ~CdRom();
    };
} // audipi

#endif //CDROM_H
