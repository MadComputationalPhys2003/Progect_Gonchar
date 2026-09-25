add_library(UrFaustCore INTERFACE)

target_include_directories(UrFaustCore
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(UrFaustCore
    INTERFACE
        cxx_std_20
)

add_executable(UrFaustConsole
    FaustSpin.cpp
)

target_link_libraries(UrFaustConsole
    PRIVATE
        UrFaustCore
)