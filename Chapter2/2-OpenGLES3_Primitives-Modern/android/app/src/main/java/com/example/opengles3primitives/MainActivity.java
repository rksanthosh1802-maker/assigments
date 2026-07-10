package com.example.opengles3primitives;

import androidx.appcompat.app.AppCompatActivity;

import android.content.res.AssetManager;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * MainActivity - Modern Android OpenGL ES 3.0 primitives demo.
 *
 * Ported from the OpenGL ES 3.0 Cookbook (Chapter 2 - OpenGLES3 Primitives).
 * Uses modern Android Studio + CMake build system, targeting API 34+.
 *
 * Touch interaction:
 *   - Tap to cycle through 7 GL primitive types:
 *     GL_POINTS, GL_LINES, GL_LINE_LOOP, GL_LINE_STRIP,
 *     GL_TRIANGLES, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN
 */
public class MainActivity extends AppCompatActivity {

    private static final String TAG = "OpenGLES3Primitives";

    static {
        System.loadLibrary("opengles3_primitives");
    }

    private GLSurfaceView glSurfaceView;
    private boolean rendererInitialized = false;

    // ---------------------------------------------------------------------------
    // JNI declarations (implemented in NativeTemplate.cpp)
    // ---------------------------------------------------------------------------
    private native boolean nativeInit(AssetManager assetManager);
    private native void    nativeResize(int width, int height);
    private native void    nativeRender();
    private native void    nativeTouchDown(float x, float y);
    private native void    nativeTouchMove(float x, float y);
    private native void    nativeTouchRelease(float x, float y);

    // ---------------------------------------------------------------------------
    // Activity lifecycle
    // ---------------------------------------------------------------------------
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        setContentView(R.layout.activity_main);

        glSurfaceView = findViewById(R.id.gl_surface_view);

        // Request OpenGL ES 3.0 context
        glSurfaceView.setEGLContextClientVersion(3);
        glSurfaceView.setRenderer(new GLSurfaceView.Renderer() {

            @Override
            public void onSurfaceCreated(GL10 gl, EGLConfig config) {
                Log.d(TAG, "onSurfaceCreated");
                boolean ok = nativeInit(getAssets());
                if (ok) {
                    rendererInitialized = true;
                    Log.d(TAG, "Native renderer initialized successfully");
                } else {
                    Log.e(TAG, "Native renderer initialization failed");
                }
            }

            @Override
            public void onSurfaceChanged(GL10 gl, int width, int height) {
                Log.d(TAG, "onSurfaceChanged: " + width + "x" + height);
                if (rendererInitialized) {
                    nativeResize(width, height);
                }
            }

            @Override
            public void onDrawFrame(GL10 gl) {
                if (rendererInitialized) {
                    nativeRender();
                }
            }
        });

        glSurfaceView.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);

        glSurfaceView.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                if (!rendererInitialized) return true;

                final float x = event.getX();
                final float y = event.getY();

                switch (event.getActionMasked()) {
                    case MotionEvent.ACTION_DOWN:
                        glSurfaceView.queueEvent(() -> nativeTouchDown(x, y));
                        break;
                    case MotionEvent.ACTION_MOVE:
                        glSurfaceView.queueEvent(() -> nativeTouchMove(x, y));
                        break;
                    case MotionEvent.ACTION_UP:
                    case MotionEvent.ACTION_CANCEL:
                        glSurfaceView.queueEvent(() -> nativeTouchRelease(x, y));
                        break;
                }
                return true;
            }
        });
    }

    @Override
    protected void onPause() {
        super.onPause();
        glSurfaceView.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        glSurfaceView.onResume();
    }
}
