cmake_minimum_required(VERSION 3.19)
project(FaustSpinGUI LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGLWidgets OpenGL Svg)

qt_add_executable(FaustSpinGUI
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)

target_link_libraries(FaustSpinGUI PRIVATE
    Qt6::Widgets
    Qt6::OpenGLWidgets
    Qt6::OpenGL
    Qt6::Svg
    FaustMath
)
