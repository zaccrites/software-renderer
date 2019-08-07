
cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

project(
    gpu
    DESCRIPTION "Graphics core implemented on an FPGA"
    VERSION 0.1.0
    LANGUAGES CXX
)


find_package(SDL2 REQUIRED)


add_executable(simulator
    "src/main.cpp"
    "src/SoftwareRenderer.cpp"
)

target_include_directories(simulator PRIVATE
    ${SDL2_INCLUDE_DIRECTORIES}
)

target_include_directories(simulator SYSTEM PUBLIC
    # TODO: Download GLM and use its CMakeLists file similar to GoogleTest
    "glm"
)

target_link_libraries(simulator
    ${SDL2_LIBRARIES}
)

target_compile_options(simulator PRIVATE
    -std=c++17
    -pedantic
    -Wall
    -Wextra
    # -Werror
    -Weffc++
    -Wshadow
    -Wcast-qual
    -Wold-style-cast
    -Wfloat-equal
    -fdiagnostics-color

    -O2
)
