cmake_minimum_required(VERSION 3.19)
project(FaustSpinGUI LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGLWidgets OpenGL Svg)

qt_add_executable(FaustSpinGUI
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    faust_spin_parameters_dialog.h
    faust_spin_parameters_dialog.cpp
    faust_spin_parameters_dialog.ui
    faust_spin_exchange_integral_dialog.h
    faust_spin_exchange_integral_dialog.cpp
    faust_spin_exchange_integral_dialog.ui
    manual_exchange_dialog_faust_spin.h
    manual_exchange_dialog_faust_spin.cpp
    manual_exchange_dialog_faust_spin.ui
    faust_spin_single_ion_anisotropy_dialog.h
    faust_spin_single_ion_anisotropy_dialog.cpp
    faust_spin_single_ion_anisotropy_dialog.ui
    manual_single_ion_anisotropy_dialog.h
    manual_single_ion_anisotropy_dialog.cpp
    manual_single_ion_anisotropy_dialog.ui
    random_single_ion_anisotropy_dialog.h random_single_ion_anisotropy_dialog.cpp random_single_ion_anisotropy_dialog.ui

)

target_link_libraries(FaustSpinGUI PRIVATE
    Qt6::Widgets
    Qt6::OpenGLWidgets
    Qt6::OpenGL
    Qt6::Svg
    FaustMath
    UrFaustWidgets
)
