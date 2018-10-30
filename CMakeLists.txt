cmake_minimum_required(VERSION 2.8.9)
project(game)

set(CMAKE_C_STANDARD 99)

file(GLOB SRC "source/*.c")
file(GLOB INC "include/*.h")

find_package(SDL2 REQUIRED)

include_directories(include)
include_directories(/usr/local/include)

link_directories(/usr/local/lib)

add_executable(game ${SRC} ${INC})

# non-standard SDL2_image lib names
if (APPLE)
    target_link_libraries(game SDL2 SDL2_Image)
else()
    target_link_libraries(game SDL2 SDL2_image)
endif()

