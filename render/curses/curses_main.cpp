#include <chrono>
#include <cstring>
#include <sstream>
#include <ncurses.h>
#include <thread>

#include "../../audipi/CdRom.h"
#include "../../audipi/Player.h"
#include "../../audipi/util.h"

extern std::string render_error(int error_num); // ugly extern

int ncurses_err(int exit_code = 0);

int curses_main() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, true);
    nodelay(stdscr, true);

    printw("Initializing...\n");
    refresh();

    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    auto cd_rom = audipi::CdRom("/dev/cdrom");

    if (!cd_rom.is_init()) {
        printw("cannot open /dev/cdrom\n");
        return ncurses_err();
    }

    if (auto drive_status_result = cd_rom.get_drive_status()) {
        if (drive_status_result.value() == audipi::drive_status::no_disk) {
            printw("No disc in disc drive!\n");
            return ncurses_err();
        }
        if (drive_status_result.value() != audipi::drive_status::disc_ok) {
            printw("Drive status not OK: ");
            printw(std::to_string(static_cast<int>(drive_status_result.value())).c_str());
            printw("\n");
            return ncurses_err();
        }
    } else {
        printw("get_drive_status: unexpected error: ");
        printw(render_error(drive_status_result.error()).c_str());
        printw("\n");
        return ncurses_err();
    }

    if (auto disk_type = cd_rom.get_disk_type(); disk_type == audipi::disk_type::unsupported) {
        printw("get_disk_type: unsupported disk\n");
        return ncurses_err();
    }

    audipi::Player player;
    if (!player.is_init()) {
        printw("Cannot initialize Player\n");
        return ncurses_err();
    }

    auto disk_toc = cd_rom.read_toc();
    if (!disk_toc) {
        printw("Cannot enqueue disk: ");
        printw(render_error(disk_toc.error()).c_str());
        printw("\n");
        return ncurses_err();
    }

    player.enqueue_cd(cd_rom, disk_toc.value());

    int keep_running = 1;
    while (keep_running) {
        player.tick();

        auto [state, current_track_index, current_track_name, current_location_in_track] = player.get_status();

        const int cur_row = getcury(stdscr);

        printw("Player status: ");
        printw(std::to_string(static_cast<int>(state)).c_str());
        printw(" - Track[");
        printw(audipi::left_pad_string(std::to_string(current_track_index), 2, '0').c_str());
        printw("]: ");
        printw(current_track_name.c_str());
        printw(" - Location: ");
        printw(msf_location_to_string(current_location_in_track).c_str());
        printw("   ");

        move(cur_row, 0);

        if (const int read_char = getch(); read_char != ERR) {
            if (read_char == 'p') {
                player.pause();
            }
            if (read_char == 'r') {
                player.play();
            }
            if (read_char == 's') {
                player.stop();
                keep_running = 0;
            }
            if (read_char == KEY_RIGHT) {
                player.next_track();
            }
            if (read_char == KEY_LEFT) {
                player.prev_track();
            }
        }

        refresh();
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    endwin();
    return 0;
}

int ncurses_err(const int exit_code) {
    printw("Press any key to exit");
    refresh();
    timeout(-1);
    getch();
    endwin();
    return exit_code;
}
