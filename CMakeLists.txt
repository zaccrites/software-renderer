
cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

project(
    gpu
    DESCRIPTION "Graphics core implemented on an FPGA"
    VERSION 0.1.0
    LANGUAGES CXX
)


find_package(SDL2 REQUIRED)






# Including this to load images for now
add_library(stb_image
    stb_image/stb_image.cpp
)
target_include_directories(stb_image PUBLIC
    stb_image
)








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
    stb_image
    ${SDL2_LIBRARIES}
)



# TODO: Do this properly
# https://stackoverflow.com/questions/7724569/debug-vs-release-in-cmake
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
    # -O2
    -g
)





# TODO: Compile various driver backends as library targets
#
#   - OpenGL translation layer
#   - Software renderer
#   - Verilated simulation
#   - USB comms to a real FPGA target
#
# These will link to an executable


# add_library(software_renderer
#     "src/SoftwareRenderer.cpp"
# )
# target_include_directories(software_renderer PUBLIC
#     # "src"
# )
