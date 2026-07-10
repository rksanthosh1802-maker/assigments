# ===========================================================================
# CMakeLists.txt – Desktop (Windows) build for OpenGLES3-Primitives-Modern
#
# Shared C++ source is in Scene/. The same Scene/ code is used by:
#   - This file          → native desktop executable (GLFW + GLEW)
#   - android/.../cpp/   → Android shared library (.so) via NDK
#   - build_web.sh       → WebAssembly/WebGL via Emscripten
#
# Usage:
#   mkdir build && cd build
#   cmake .. -G "Visual Studio 17 2022" -A x64
#   cmake --build . --config Debug
# ===========================================================================

cmake_minimum_required(VERSION 3.15)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

cmake_policy(VERSION 3.15)

project(OpenGLES3Primitives)

set(CMAKE_CXX_STANDARD          17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch GLFW + GLEW for the desktop build
include(ImportDependencies)
importDependencies()

add_definitions(-DPLATFORM_WINDOWS -DGLEW_STATIC)

# ---------------------------------------------------------------------------
# Source files – everything in Scene/ (shared with Android + WebGL)
# ---------------------------------------------------------------------------
set(SCENE_DIR ${CMAKE_CURRENT_LIST_DIR}/Scene)
file(GLOB SCENE_SRC "${SCENE_DIR}/*.cpp")

add_executable(OpenGLES3Primitives ${SCENE_SRC})

target_include_directories(OpenGLES3Primitives PRIVATE ${SCENE_DIR})

target_link_libraries(OpenGLES3Primitives
    PRIVATE glfw
    PRIVATE libglew_static
)

# C++17 on MSVC
if(MSVC)
    target_compile_options(OpenGLES3Primitives PRIVATE /W4 /WX- /permissive- /std:c++17)
endif()

# ---------------------------------------------------------------------------
# Post-build: copy the GLSL shaders so they are reachable at
#   <build_dir>/assets/shader/<name>   (same relative path used in ShaderHelper.h)
# ---------------------------------------------------------------------------
add_custom_command(TARGET OpenGLES3Primitives POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_LIST_DIR}/android/app/src/main/assets"
        "$<TARGET_FILE_DIR:OpenGLES3Primitives>/assets"
    COMMENT "Copying shader assets to build directory"
)

# Tell Visual Studio to use the exe's output directory as the working
# directory when debugging, so the relative "assets/shader/" paths work.
set_target_properties(OpenGLES3Primitives PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:OpenGLES3Primitives>"
)
