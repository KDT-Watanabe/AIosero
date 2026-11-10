cmake_minimum_required(VERSION 3.20)
project(sandwich_othello LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Engine core: no DxLib dependency (research.md #1, SC-006).
add_library(othello_engine STATIC
    src/engine/board.cpp
    src/engine/rules.cpp
    src/engine/ai.cpp
)
target_include_directories(othello_engine PUBLIC src/engine)

add_executable(engine_tests tests/engine_tests.cpp)
if (MSVC)
    target_compile_options(engine_tests PRIVATE /utf-8)
    target_compile_options(othello_engine PRIVATE /utf-8)
endif()
target_link_libraries(engine_tests PRIVATE othello_engine)

# App layer links DxLib only when available; engine builds without it.
if (EXISTS "${CMAKE_SOURCE_DIR}/third_party/dxlib/DxLibNoneModel_NoneSoftDrawCode_VC/DxLib.h")
    set(DXLIB_DIR "${CMAKE_SOURCE_DIR}/third_party/dxlib/DxLibNoneModel_NoneSoftDrawCode_VC")
    add_executable(sandwich_othello WIN32 src/app/main.cpp src/app/render.cpp src/app/input.cpp src/app/sound.cpp)
    target_include_directories(sandwich_othello PRIVATE src/engine src/app "${DXLIB_DIR}")
    target_link_libraries(sandwich_othello PRIVATE othello_engine)
    if (MSVC)
        # Japanese UI via wchar_t: DXLib's TCHAR maps to wchar_t under UNICODE.
        target_compile_definitions(sandwich_othello PRIVATE UNICODE _UNICODE)
        # /wd4828: DxLib headers are Shift-JIS; their comments trip the UTF-8 decoder.
        target_compile_options(sandwich_othello PRIVATE /utf-8 /wd4828 /wd4010)
        # DxLib auto-links the right CRT variant (vs2015+ ABI, Debug/Release, W) via pragma.
        target_link_directories(sandwich_othello PRIVATE "${DXLIB_DIR}")
        target_link_libraries(sandwich_othello PRIVATE
            kernel32 user32 gdi32 advapi32 shell32 ole32 oleaut32 uuid winmm)
    endif()
    # Sound effects are loaded from assets/ next to the exe.
    add_custom_command(TARGET sandwich_othello POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/assets" "$<TARGET_FILE_DIR:sandwich_othello>/assets")
endif()

enable_testing()
add_test(NAME engine_tests COMMAND engine_tests)
