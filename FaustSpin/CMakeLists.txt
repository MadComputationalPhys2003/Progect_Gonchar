cmake_minimum_required(VERSION 3.21)

project(UrFaust LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

add_subdirectory(FaustSpin)
add_subdirectory(UrFaustGUI_TEST_1)