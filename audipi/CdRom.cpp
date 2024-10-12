#include "CdRom.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/cdrom.h>

constexpr __u8 CD_TIME_FORMAT = CDROM_MSF;

namespace audipi {
    CdRom::CdRom(const std::string &fd_path) {
        this->cdrom_fd = open("/dev/cdrom", O_RDONLY | O_NONBLOCK);
    }

    bool CdRom::is_init() const {
        return this->cdrom_fd >= 0;
    }

    std::expected<void, int> CdRom::start() const {
        int error = ioctl(cdrom_fd, CDROMSTART, 0);
        if (error != 0) {
            return std::unexpected(errno);
        }
        return {};
    }

    std::expected<void, int> CdRom::stop() const {
        int error = ioctl(cdrom_fd, CDROMSTOP, 0);
        if (error != 0) {
            return std::unexpected(errno);
        }
        return {};
    }

    std::expected<void, int> CdRom::eject() const {
        int error = ioctl(cdrom_fd, CDROMEJECT, 0);

        // if (eject_error != 0 and errno == ENOSYS) {
        //     return std::unexpected(eject_error::cannot_eject);
        // }
        //
        // if (eject_error != 0 and errno == EBUSY) {
        //     return std::unexpected(eject_error::device_busy);
        // }
        //
        // if (eject_error != 0) {
        //     return std::unexpected(eject_error::undefined);
        // }

        if (error != 0) {
            return std::unexpected(errno << 16 | error);
        }

        return {};
    }

    std::expected<void, int> CdRom::close_tray() const {
        int error = ioctl(cdrom_fd, CDROMCLOSETRAY, 0);

        if (error != 0) {
            return std::unexpected(errno << 16 | error);
        }

        return {};
    }

    std::expected<drive_status, int> CdRom::get_drive_status() const {
        int drive_status = ioctl(cdrom_fd, CDROM_DRIVE_STATUS, 0);

        if (drive_status == static_cast<int>(drive_status::error)) {
            return std::unexpected(errno);
        }

        return static_cast<audipi::drive_status>(drive_status);
    }

    disk_type CdRom::get_disk_type() const {
        int disk_status = ioctl(cdrom_fd, CDROM_DISC_STATUS, 0);

        if (disk_status != CDS_AUDIO and disk_status != CDS_MIXED) {
            return disk_type::unsupported;
        }

        return static_cast<disk_type>(disk_status);
    }

    std::expected<audio_disk_toc, int> CdRom::read_toc() const {
        cdrom_tochdr header{};

        int header_error = ioctl(cdrom_fd, CDROMREADTOCHDR, &header);

        if (header_error != 0) {
            return std::unexpected(header_error);
        }

        std::vector<audio_disk_toc_entry> entries;

        for (auto track = header.cdth_trk0; track < header.cdth_trk1; ++track) {
            cdrom_tocentry entry {
                .cdte_track = track,
                .cdte_format = CD_TIME_FORMAT
            };

            int entry_error = ioctl(cdrom_fd, CDROMREADTOCENTRY, &entry);
            if (entry_error != 0) {
                return std::unexpected(entry_error);
            }

            entries.push_back({
                .track_num = track,
                .address = msf_location{
                    entry.cdte_addr.msf.minute,
                    entry.cdte_addr.msf.second,
                    entry.cdte_addr.msf.frame
                }
            });
        }

        return audio_disk_toc{header.cdth_trk0, header.cdth_trk1, entries};
    }

    CdRom::~CdRom() {
        if (is_init()) {
            close(cdrom_fd);
        }
    }
}