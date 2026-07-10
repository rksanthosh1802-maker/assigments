# =============================================================================
# CMakeLists.txt - Desktop (Windows) build for AndroidProgrammingQuiz-CrossPlatformCI
#
# Scene/ holds the shared Model + Renderer architecture used by Desktop and
# Web. Android builds through android/app/src/main/cpp/CMakeLists.txt instead,
# using NativeTemplate.cpp as its JNI entry point.
#
# Usage:
#   cmake -S . -B build_desktop -G "Visual Studio 17 2022" -A x64
#   cmake --build build_desktop --config Debug
# =============================================================================

cmake_minimum_required(VERSION 3.15)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
cmake_policy(VERSION 3.15)

project(AndroidProgrammingQuiz)

set(CMAKE_CXX_STANDARD          17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(ImportDependencies)
importDependencies()

add_definitions(-DPLATFORM_WINDOWS -DGLEW_STATIC)

set(SCENE_DIR ${CMAKE_CURRENT_LIST_DIR}/Scene)
# Glob on purpose - any new Model .cpp dropped into Scene/ is picked up
# automatically without touching this file.
file(GLOB SCENE_SRC "${SCENE_DIR}/*.cpp")

add_executable(AndroidProgrammingQuiz ${SCENE_SRC})

target_include_directories(AndroidProgrammingQuiz PRIVATE ${SCENE_DIR})

target_link_libraries(AndroidProgrammingQuiz
    PRIVATE glfw
    PRIVATE libglew_static
)

if(MSVC)
    target_compile_options(AndroidProgrammingQuiz PRIVATE /W4 /WX- /permissive- /std:c++17)
endif()

# Triangle/Square now read their .glsl source from disk at runtime, so the
# assets folder has to sit next to the built executable.
add_custom_command(TARGET AndroidProgrammingQuiz POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_LIST_DIR}/android/app/src/main/assets"
        "$<TARGET_FILE_DIR:AndroidProgrammingQuiz>/assets"
    COMMENT "Copying shader assets to build directory"
)

set_target_properties(AndroidProgrammingQuiz PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:AndroidProgrammingQuiz>"
)
