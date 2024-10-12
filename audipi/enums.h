#ifndef ENUMS_H
#define ENUMS_H

namespace audipi {
    enum class drive_status {
        no_info = CDS_NO_INFO,
        no_disk = CDS_NO_DISC,
        tray_open = CDS_TRAY_OPEN,
        drive_not_ready = CDS_DRIVE_NOT_READY,
        disc_ok = CDS_DISC_OK,
        error = -1
    };

    enum class disk_type {
        audio = CDS_AUDIO,
        mixed = CDS_MIXED,
        unsupported = -1
    };
}

#endif //ENUMS_H
