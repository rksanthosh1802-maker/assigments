/**
 * @file    ShaderHelper.h
 * @brief   Single place shader-loading logic lives for every Model in the
 *          project. 
 * @author Santhosh Ravikumar
 * @date    2026
 */
#pragma once
#include "Platform.h"
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#endif

class ShaderHelper {
public:
    // --- Core compile/link, shared by both loading methods below ---
    static GLuint compileShader(GLenum type, const char* src);
    static GLuint linkProgram(GLuint vert, GLuint frag);

    // --- Method 1: "stringify" - build directly from inline source strings ---
    static GLuint buildProgram(const char* vertSrc, const char* fragSrc);

    // --- Method 2: file-based - load shader source from disk / APK assets ---
#ifdef PLATFORM_ANDROID
    static std::string loadAsset(AAssetManager* mgr, const char* path);
    static GLuint buildProgramFromAssets(AAssetManager* mgr, const char* vertPath, const char* fragPath);
#else
    static std::string loadFile(const char* filename);
    static GLuint buildProgramFromFile(const char* vertFile, const char* fragFile);
#endif
};

// ---------------------------------------------------------------------------
// Core compile / link
// ---------------------------------------------------------------------------

inline GLuint ShaderHelper::compileShader(GLenum type, const char* src)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint ok = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (ok == GL_FALSE) {
        GLint len = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
        std::vector<char> log(len > 1 ? len : 1);
        glGetShaderInfoLog(shader, len, nullptr, log.data());
        LOGE("Shader compile failed: %s", log.data());
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

inline GLuint ShaderHelper::linkProgram(GLuint vert, GLuint frag)
{
    GLuint program = glCreateProgram();
    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glLinkProgram(program);

    GLint ok = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (ok == GL_FALSE) {
        GLint len = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
        std::vector<char> log(len > 1 ? len : 1);
        glGetProgramInfoLog(program, len, nullptr, log.data());
        LOGE("Program link failed: %s", log.data());
        glDeleteProgram(program);
        program = 0;
    }

    glDeleteShader(vert);
    glDeleteShader(frag);
    return program;
}

// ---------------------------------------------------------------------------
// Method 1: stringify
// ---------------------------------------------------------------------------

inline GLuint ShaderHelper::buildProgram(const char* vertSrc, const char* fragSrc)
{
    GLuint vert = compileShader(GL_VERTEX_SHADER, vertSrc);
    GLuint frag = compileShader(GL_FRAGMENT_SHADER, fragSrc);
    if (vert == 0 || frag == 0) {
        if (vert) glDeleteShader(vert);
        if (frag) glDeleteShader(frag);
        return 0;
    }
    return linkProgram(vert, frag);
}

// ---------------------------------------------------------------------------
// Method 2: file-based
// ---------------------------------------------------------------------------

#ifdef PLATFORM_ANDROID

inline std::string ShaderHelper::loadAsset(AAssetManager* mgr, const char* path)
{
    if (mgr == nullptr) {
        LOGE("loadAsset: null AAssetManager for %s", path);
        return std::string();
    }

    AAsset* asset = AAssetManager_open(mgr, path, AASSET_MODE_BUFFER);
    if (asset == nullptr) {
        LOGE("loadAsset: could not open %s", path);
        return std::string();
    }

    size_t size = AAsset_getLength(asset);
    std::string contents(size, '\0');
    AAsset_read(asset, contents.data(), size);
    AAsset_close(asset);
    return contents;
}

inline GLuint ShaderHelper::buildProgramFromAssets(AAssetManager* mgr, const char* vertPath, const char* fragPath)
{
    std::string vertSrc = loadAsset(mgr, vertPath);
    std::string fragSrc = loadAsset(mgr, fragPath);
    if (vertSrc.empty() || fragSrc.empty())
        return 0;
    return buildProgram(vertSrc.c_str(), fragSrc.c_str());
}

#else

inline std::string ShaderHelper::loadFile(const char* filename)
{
    static const char* kSearchDirs[] = {
        "assets/shader/",
        "",
        "shader/",
        "assets/"
    };

    for (const char* dir : kSearchDirs) {
        std::string path = std::string(dir) + filename;
        std::ifstream file(path);
        if (file.is_open()) {
            std::ostringstream ss;
            ss << file.rdbuf();
            return ss.str();
        }
    }

    LOGE("loadFile: could not find %s in any known shader directory", filename);
    return std::string();
}

inline GLuint ShaderHelper::buildProgramFromFile(const char* vertFile, const char* fragFile)
{
    std::string vertSrc = loadFile(vertFile);
    std::string fragSrc = loadFile(fragFile);
    if (vertSrc.empty() || fragSrc.empty())
        return 0;
    return buildProgram(vertSrc.c_str(), fragSrc.c_str());
}

#endif
