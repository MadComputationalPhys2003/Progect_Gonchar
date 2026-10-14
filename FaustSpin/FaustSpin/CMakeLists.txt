add_library(UrFaustCore STATIC
    "Faust math/Faust_Matrix.cpp"
)

target_include_directories(UrFaustCore
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}/Subprograms"
)

target_compile_features(UrFaustCore PUBLIC cxx_std_20)

add_executable(UrFaustConsole
    FaustSpin.cpp
    Subprograms/UrFaustProgram.h
)

target_link_libraries(UrFaustConsole PRIVATE UrFaustCore)