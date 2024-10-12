#include <iostream>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/cdrom.h>

int main(int argc, char *argv[]) {

    int cdrom_fd = open("/dev/cdrom", O_RDONLY | O_NONBLOCK);

    if (cdrom_fd < 0) {
        std::cerr << "cannot open /dev/cdrom" << std::endl;
        return -1;
    }

    int eject_error = ioctl(cdrom_fd, CDROMEJECT, 0);

    if (eject_error == ENOSYS) {
        std::cerr << "/dev/cdrom cannot eject" << std::endl;
        return -1;
    }

    if (eject_error == EBUSY) {
        std::cerr << "/dev/cdrom in use or similar" << std::endl;
        return -1;
    }

    return 0;
}
