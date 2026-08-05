cmake_minimum_required(VERSION 3.22.1)

project("fan3d")

# Enforce C++17 across all targets
set(CMAKE_CXX_STANDARD          17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Android-only guard
if(NOT ANDROID)
    message(FATAL_ERROR
        "This CMakeLists.txt is for Android builds only. "
        "Use the platform-specific build system for other targets.")
endif()

add_definitions(-DANDROID -DPLATFORM_ANDROID)

# ---------------------------------------------------------------------------
# Shared Scene sources live in the project root (5 levels up from this file):
#   android/app/src/main/cpp/CMakeLists.txt → project root → Scene/
# ---------------------------------------------------------------------------
set(SCENE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../Scene)

# ---------------------------------------------------------------------------
# GLM (header-only) - required by Scene/Transform.h
# ---------------------------------------------------------------------------
include(FetchContent)
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
)
FetchContent_MakeAvailable(glm)

add_library(fan3d SHARED
    # Android JNI bridge
    NativeTemplate.cpp

    # Shared rendering code
    ${SCENE_DIR}/Renderer.cpp
    ${SCENE_DIR}/Transform.cpp
    ${SCENE_DIR}/Fan.cpp
)

# ---------------------------------------------------------------------------
# Include paths
# ---------------------------------------------------------------------------
target_include_directories(fan3d PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}   # NativeTemplate.cpp sits here
    ${SCENE_DIR}                  # All shared Scene headers
)

# ---------------------------------------------------------------------------
# System libraries provided by the Android NDK
# ---------------------------------------------------------------------------
find_library(log-lib    log)
find_library(android-lib android)
find_library(glesv3-lib GLESv3)
find_library(egl-lib    EGL)

target_link_libraries(fan3d
    glm::glm
    ${log-lib}
    ${android-lib}
    ${glesv3-lib}
    ${egl-lib}
)