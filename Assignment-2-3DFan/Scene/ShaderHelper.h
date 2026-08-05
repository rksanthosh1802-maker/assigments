#pragma once

/**
 * ShaderHelper.h
 *
 * Self-contained helper for compiling GLSL shaders and linking programs.
 * Supports three platforms:
 *   PLATFORM_ANDROID     – shader source loaded from APK assets via AAssetManager
 *   PLATFORM_WINDOWS     – shader source loaded from the filesystem (std::ifstream)
 *   PLATFORM_EMSCRIPTEN  – shader source loaded from the Emscripten virtual filesystem
 */

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

#ifdef PLATFORM_ANDROID
    static std::string loadAsset(AAssetManager* mgr, const char* path) {
        if (!mgr || !path) return {};
        AAsset* asset = AAssetManager_open(mgr, path, AASSET_MODE_BUFFER);
        if (!asset) {
            LOGE("ShaderHelper: cannot open asset: %s", path);
            return {};
        }
        size_t size = static_cast<size_t>(AAsset_getLength(asset));
        std::string src(size, '\0');
        AAsset_read(asset, &src[0], size);
        AAsset_close(asset);
        return src;
    }

    /**
     * Build a GL program by loading shaders from APK assets.
     * vertPath / fragPath are relative to the assets root,
     * e.g. "shader/TriangleVertex.glsl".
     */
    static GLuint buildProgramFromAssets(AAssetManager* mgr,
                                         const char* vertPath,
                                         const char* fragPath) {
        std::string vertSrc = loadAsset(mgr, vertPath);
        std::string fragSrc = loadAsset(mgr, fragPath);
        if (vertSrc.empty() || fragSrc.empty()) return 0;
        return buildProgram(vertSrc.c_str(), fragSrc.c_str());
    }
#endif // PLATFORM_ANDROID

#if defined(PLATFORM_WINDOWS) || defined(PLATFORM_EMSCRIPTEN)
    static std::string loadFile(const char* filename) {
        const std::vector<std::string> searchPaths = {
            std::string("assets/shader/") + filename,
            std::string(filename),
            std::string("shader/") + filename,
            std::string("assets/") + filename,
        };
        for (const auto& path : searchPaths) {
            std::ifstream f(path);
            if (f.is_open()) {
                std::stringstream buf;
                buf << f.rdbuf();
                LOGI("ShaderHelper: loaded %s from %s", filename, path.c_str());
                return buf.str();
            }
        }
        LOGE("ShaderHelper: cannot find %s", filename);
        return {};
    }

    static GLuint buildProgramFromFile(const char* vertFilename,
                                       const char* fragFilename) {
        std::string vertSrc = loadFile(vertFilename);
        std::string fragSrc = loadFile(fragFilename);
        if (vertSrc.empty() || fragSrc.empty()) return 0;
        return buildProgram(vertSrc.c_str(), fragSrc.c_str());
    }
#endif // PLATFORM_WINDOWS || PLATFORM_EMSCRIPTEN

    // -----------------------------------------------------------------------
    // Core GL operations (all platforms)
    // -----------------------------------------------------------------------

    /** Compile a single shader from source. Returns 0 on failure. */
    static GLuint compileShader(GLenum type, const char* src) {
        GLuint shader = glCreateShader(type);
        if (!shader) { LOGE("ShaderHelper: glCreateShader failed"); return 0; }
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);

        GLint ok = GL_FALSE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
        if (!ok) {
            GLint len = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
            std::string log(len, '\0');
            glGetShaderInfoLog(shader, len, nullptr, &log[0]);
            LOGE("ShaderHelper: compile error:\n%s", log.c_str());
            glDeleteShader(shader);
            return 0;
        }
        return shader;
    }

    /** Link a vertex + fragment shader into a GL program. Returns 0 on failure. */
    static GLuint linkProgram(GLuint vert, GLuint frag) {
        GLuint prog = glCreateProgram();
        glAttachShader(prog, vert);
        glAttachShader(prog, frag);
        glLinkProgram(prog);

        GLint ok = GL_FALSE;
        glGetProgramiv(prog, GL_LINK_STATUS, &ok);
        if (!ok) {
            GLint len = 0;
            glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
            std::string log(len, '\0');
            glGetProgramInfoLog(prog, len, nullptr, &log[0]);
            LOGE("ShaderHelper: link error:\n%s", log.c_str());
            glDeleteProgram(prog);
            return 0;
        }
        glDetachShader(prog, vert);
        glDetachShader(prog, frag);
        glDeleteShader(vert);
        glDeleteShader(frag);
        return prog;
    }

    /** Compile vertex + fragment sources and link into a program. */
    static GLuint buildProgram(const char* vertSrc, const char* fragSrc) {
        GLuint vert = compileShader(GL_VERTEX_SHADER,   vertSrc);
        if (!vert) return 0;
        GLuint frag = compileShader(GL_FRAGMENT_SHADER, fragSrc);
        if (!frag) { glDeleteShader(vert); return 0; }
        return linkProgram(vert, frag);
    }
};
