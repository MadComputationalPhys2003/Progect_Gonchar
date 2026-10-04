cmake_minimum_required(VERSION 3.19)
project(UrFaustGUI_TEST_1 LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS OpenGLWidgets OpenGL)

qt_standard_project_setup()

qt_add_executable(UrFaustGUI_TEST_1
    WIN32 MACOSX_BUNDLE
    main.cpp
    main_window_1.cpp
    main_window_1.h
    main_window_1.ui
    parameters_dialog.h parameters_dialog.cpp parameters_dialog.ui
    system_results_dialog.h system_results_dialog.cpp system_results_dialog.ui
    energy_plot_dialog.h energy_plot_dialog.cpp energy_plot_dialog.ui
    spin_visualization_dialog.h spin_visualization_dialog.cpp spin_visualization_dialog.ui
    spin_opengl_widget.h spin_opengl_widget.cpp
    cats.qrc
)
set_target_properties(UrFaustGUI_TEST_1 PROPERTIES
    AUTORCC ON
)
target_link_libraries(UrFaustGUI_TEST_1
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt6::OpenGLWidgets
        Qt6::OpenGL
        UrFaustCore
)

install(TARGETS UrFaustGUI_TEST_1
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET UrFaustGUI_TEST_1
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
