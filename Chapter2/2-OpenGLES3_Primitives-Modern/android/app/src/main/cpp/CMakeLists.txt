cmake_minimum_required(VERSION 3.22.1)

project("opengles3_primitives")

set(CMAKE_CXX_STANDARD          17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

add_library(opengles3_primitives SHARED
    # Android JNI bridge
    NativeTemplate.cpp

    # Shared rendering code
    ${SCENE_DIR}/Renderer.cpp
    ${SCENE_DIR}/Primitives.cpp
)

# ---------------------------------------------------------------------------
# Include paths
# ---------------------------------------------------------------------------
target_include_directories(opengles3_primitives PRIVATE
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

target_link_libraries(opengles3_primitives
    ${log-lib}
    ${android-lib}
    ${glesv3-lib}
    ${egl-lib}
)
