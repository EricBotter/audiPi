cmake_minimum_required(VERSION 3.25)
project(audipi)

set(CMAKE_CXX_STANDARD 23)

find_package(ALSA REQUIRED)

set(CURSES_NEED_NCURSES TRUE)
find_package(Curses REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
qt_standard_project_setup()

add_compile_definitions(AUDIPI_DEBUG=0)
add_compile_options(-Wall -Werror)

add_library(render_curses render/curses/curses_main.cpp)
add_library(render_qt render/qt/qt_main.cpp render/qt/main_window.ui
        render/qt/main_window.cpp)

add_library(audipi_lib
        audipi/CdRom.cpp
        audipi/Player.cpp
        audipi/structs.cpp
        audipi/AudioDevice.cpp
        audipi/SampleBuffer.cpp
        audipi/PlayerTrack.cpp
        audipi/util.cpp)

add_executable(audipi main.cpp main.cpp)

target_link_libraries(render_curses PRIVATE ${CURSES_LIBRARIES})
target_link_libraries(render_qt PRIVATE Qt6::Core Qt6::Widgets)

target_link_libraries(audipi PRIVATE audipi_lib render_curses render_qt ALSA::ALSA)