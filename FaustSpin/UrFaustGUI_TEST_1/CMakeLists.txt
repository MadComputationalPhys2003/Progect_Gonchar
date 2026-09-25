cmake_minimum_required(VERSION 3.19)
project(UrFaustGUI_TEST_1 LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(UrFaustGUI_TEST_1
    WIN32 MACOSX_BUNDLE
    main.cpp
    main_window_1.cpp
    main_window_1.h
    main_window_1.ui
)

target_link_libraries(UrFaustGUI_TEST_1
    PRIVATE
        Qt::Core
        Qt::Widgets
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
