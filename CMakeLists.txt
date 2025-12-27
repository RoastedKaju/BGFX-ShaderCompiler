cmake_minimum_required(VERSION 3.24)

set(CMAKE_TOOLCHAIN_FILE
    "${CMAKE_SOURCE_DIR}/vendor/vcpkg/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING ""
)
set(VCPKG_MANIFEST_MODE ON)

project(OfflineShaderCompiler CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} "src/main.cpp")

find_package(bgfx CONFIG REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE
    bgfx::bx
    bgfx::bgfx
    bgfx::bimg
    bgfx::bimg_decode)