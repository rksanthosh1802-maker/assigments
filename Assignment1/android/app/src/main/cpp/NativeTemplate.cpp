/**
 * @file    NativeTemplate.cpp

 * @author  Santhosh Ravikumar
 * @date    2026
 */
#define LOG_TAG "AndroidProgrammingQuizNative"

#include <jni.h>
#include <android/asset_manager_jni.h>

#include "Renderer.h"

extern "C" JNIEXPORT jboolean JNICALL
Java_com_example_helloworldandroid_MainActivity_nativeInit(
        JNIEnv* env, jobject /*thiz*/, jobject assetManager)
{
    LOGI("nativeInit called");

    AAssetManager* mgr = AAssetManager_fromJava(env, assetManager);
    Renderer::Instance().setAssetManager(mgr);

    return Renderer::Instance().initializeRenderer() ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_helloworldandroid_MainActivity_nativeResize(
        JNIEnv* /*env*/, jobject /*thiz*/, jint width, jint height)
{
    Renderer::Instance().resize(static_cast<int>(width), static_cast<int>(height));
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_helloworldandroid_MainActivity_nativeRender(
        JNIEnv* /*env*/, jobject /*thiz*/)
{
    Renderer::Instance().render();
}
