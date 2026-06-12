cmake_minimum_required(VERSION 3.15)
project(BrownianMotionPBD CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(brownian_motion ${SOURCES})

# --- РАСКОММЕНТИРОВАНО ДЛЯ ПОДКЛЮЧЕНИЯ ГРАФИКИ ---
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)
if(SFML_FOUND)
    target_link_libraries(brownian_motion sfml-graphics sfml-window sfml-system)
endif()
