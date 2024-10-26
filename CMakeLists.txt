cmake_minimum_required(VERSION 3.29)
project(audipi)

set(CMAKE_CXX_STANDARD 23)

find_package(ALSA REQUIRED)

add_executable(audipi main.cpp
        main.cpp
        audipi/CdRom.cpp
        audipi/Player.cpp
        audipi/structs.cpp
        audipi/AudioDevice.cpp
        audipi/SampleBuffer.cpp
        audipi/PlayerTrack.cpp)

target_link_libraries(audipi ALSA::ALSA)