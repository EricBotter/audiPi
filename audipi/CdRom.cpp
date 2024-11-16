#include "CdRom.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/cdrom.h>

constexpr __u8 CD_TIME_FORMAT = CDROM_MSF;

audipi::msf_location cdrom_addr_to_msf_location(const cdrom_addr &addr) {
    return {
        .minute = addr.msf.minute,
        .second = addr.msf.second,
        .frame = addr.msf.frame
    };
}

cdrom_addr msf_location_to_cdrom_addr(const audipi::msf_location &location) {
    return {
        .msf = {
            .minute = location.minute,
            .second = location.second,
            .frame = location.frame
        }
    };
}

namespace audipi {
    CdRom::CdRom(const std::string &fd_path) {
        this->cdrom_fd = open(fd_path.c_str(), O_RDONLY | O_NONBLOCK);
    }

    bool CdRom::is_init() const {
        return this->cdrom_fd >= 0;
    }

    std::expected<void, int> CdRom::start() const {
        if (int error = ioctl(cdrom_fd, CDROMSTART, 0)) {
            return std::unexpected(errno);
        }
        return {};
    }

    std::expected<void, int> CdRom::stop() const {
        if (int error = ioctl(cdrom_fd, CDROMSTOP, 0)) {
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
        if (int error = ioctl(cdrom_fd, CDROMCLOSETRAY, 0)) {
            return std::unexpected(error);
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

    std::expected<disk_toc, int> CdRom::read_toc() const {
        cdrom_tochdr header{};

        if (int header_error = ioctl(cdrom_fd, CDROMREADTOCHDR, &header)) {
            return std::unexpected(header_error);
        }

        std::vector<disk_toc_entry> entries;

        cdrom_tocentry entry{
            .cdte_track = 0,
            .cdte_format = CD_TIME_FORMAT
        };

        for (auto track = header.cdth_trk0; track <= header.cdth_trk1; ++track) {
            entry.cdte_track = track;

            if (const int entry_error = ioctl(cdrom_fd, CDROMREADTOCENTRY, &entry)) {
                return std::unexpected(entry_error);
            }

            const auto current_msf = cdrom_addr_to_msf_location(entry.cdte_addr);

            if (!entries.empty()) {
                entries.back().duration = current_msf - entries.back().address;
            }

            entries.push_back({
                .track_num = track,
                .address = current_msf
            });
        }

        // read leadout track to determine duration of last track
        entry.cdte_track = CDROM_LEADOUT;

        if (const int entry_error = ioctl(cdrom_fd, CDROMREADTOCENTRY, &entry); entry_error == 0) {
            const auto current_msf = cdrom_addr_to_msf_location(entry.cdte_addr);

            entries.back().duration = current_msf - entries.back().address;
        } else {
            // do CDs always have a leadout track?
            printf("CDROMREADTOCENTRY error on reading leadout track: %d\n", entry_error);
        }

        return disk_toc{header.cdth_trk0, header.cdth_trk1, entries};
    }

    std::expected<audio_frame, int> CdRom::read_frame(const msf_location &location) const {

        u_int8_t buffer[2352]{};

        cdrom_read_audio audio_read{
            .addr = msf_location_to_cdrom_addr(location),
            .addr_format = CD_TIME_FORMAT,
            .nframes = 1,
            .buf = buffer
        };

        if (int read_error = ioctl(cdrom_fd, CDROMREADAUDIO, &audio_read)) {
            return std::unexpected(read_error);
        }

        audio_frame frame{};

        frame.raw_data = std::to_array(buffer);

        cdrom_subchnl audio_subchannel{
            .cdsc_format = CDROM_MSF,
        };

        if (int read_error = ioctl(cdrom_fd, CDROMSUBCHNL, &audio_subchannel)) {
            return std::unexpected(read_error);
        }

        frame.track_num = audio_subchannel.cdsc_trk;
        frame.index_num = audio_subchannel.cdsc_ind;
        frame.location_abs = cdrom_addr_to_msf_location(audio_subchannel.cdsc_absaddr);
        frame.location_rel = cdrom_addr_to_msf_location(audio_subchannel.cdsc_reladdr);

        return frame;
    }

    CdRom::~CdRom() {
        if (is_init()) {
            close(cdrom_fd);
        }
    }
}
