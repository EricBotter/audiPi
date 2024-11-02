cmake_minimum_required(VERSION 3.25)
project(audipi)

set(CMAKE_CXX_STANDARD 23)

find_package(ALSA REQUIRED)

set(CURSES_NEED_NCURSES TRUE)
find_package(Curses REQUIRED)

add_compile_definitions(AUDIPI_DEBUG=0)

add_library(render_curses render/curses/curses_main.cpp)

add_library(audipi_lib
        audipi/CdRom.cpp
        audipi/Player.cpp
        audipi/structs.cpp
        audipi/AudioDevice.cpp
        audipi/SampleBuffer.cpp
        audipi/PlayerTrack.cpp
        audipi/util.cpp)

add_executable(audipi main.cpp main.cpp)

target_link_libraries(render_curses ${CURSES_LIBRARIES})

target_link_libraries(audipi audipi_lib render_curses ALSA::ALSA)