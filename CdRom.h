#ifndef CDROM_H
#define CDROM_H
#include <expected>
#include <string>
#include <linux/cdrom.h>

namespace audipi {
    enum class drive_status {
        no_info = CDS_NO_INFO,
        no_disk = CDS_NO_DISC,
        tray_open = CDS_TRAY_OPEN,
        drive_not_ready = CDS_DRIVE_NOT_READY,
        disc_ok = CDS_DISC_OK,
        error = -1
    };

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

        ~CdRom();
    };
} // audipi

#endif //CDROM_H
