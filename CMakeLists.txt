cmake_minimum_required(VERSION 3.21)

# ──────────────────────────────────────────────────────────────
# IMPORTANT: vcpkg toolchain must be set BEFORE project()
# Either set it here (hardcoded path) OR pass it via CLion's
# CMake settings: -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
# ──────────────────────────────────────────────────────────────
# Uncomment and adjust this line if you don't set it in CLion settings:
# set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")

project(StoreManagementGUI)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ──────────────────────────────────────────────────────────────
# Find Slint (installed via: vcpkg install slint)
# No Rust build, no FetchContent, no download on every reload.
# ──────────────────────────────────────────────────────────────
find_package(Slint REQUIRED
        PATHS "C:/Program Files/Slint-cpp 1.15.1/lib/cmake/Slint"
        NO_DEFAULT_PATH
)

# ──────────────────────────────────────────────────────────────
# Your executable
# ──────────────────────────────────────────────────────────────
add_executable(store_gui
        main.cpp
        Store.cpp
        GuiController.cpp
)

# ──────────────────────────────────────────────────────────────
# Process the .slint UI file and link
# ──────────────────────────────────────────────────────────────
slint_target_sources(store_gui gui/appwindow.slint)
target_link_libraries(store_gui PRIVATE Slint::Slint)