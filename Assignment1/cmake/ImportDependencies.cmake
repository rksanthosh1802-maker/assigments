# =============================================================================
# ImportDependencies.cmake
#
# Fetches GLFW + GLEW for the Desktop build. Android has its own GLES3/EGL
# libs and Web gets its GL bindings from Emscripten, so both skip this.
# =============================================================================

include(FetchContent)

function(importDependencies)

    if(ANDROID)
        return()
    endif()

    message(STATUS "[ImportDependencies] Fetching GLFW ...")
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG        master
        GIT_SHALLOW    TRUE
    )
    set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(glfw)

    message(STATUS "[ImportDependencies] Fetching GLEW ...")
    FetchContent_Declare(
        glew
        GIT_REPOSITORY https://github.com/Perlmint/glew-cmake.git
        GIT_TAG        master
        GIT_SHALLOW    TRUE
    )
    set(glew-cmake_BUILD_SHARED OFF CACHE BOOL "" FORCE)
    set(glew-cmake_BUILD_STATIC ON  CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(glew)

    message(STATUS "[ImportDependencies] All desktop dependencies ready.")

endfunction()
