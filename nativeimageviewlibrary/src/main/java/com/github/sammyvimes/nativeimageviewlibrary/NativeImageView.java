package com.github.sammyvimes.nativeimageviewlibrary;

import android.content.Context;
import android.content.res.AssetManager;
import android.graphics.PixelFormat;
import android.opengl.GLSurfaceView;
import android.os.SystemClock;
import android.util.AttributeSet;
import android.util.Log;
import android.view.SurfaceView;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * Created by Semyon on 04.01.2016.
 */
public class NativeImageView extends GLSurfaceView {

    private static native void native_start();
    private static native void native_surface_created();
    private static native void native_gl_resize(int w, int h);
    private static native void native_gl_render();

    private String imagePath;
    private NativeAssetManager nativeAssetManager;

    private Context context;

    public NativeImageView(final Context context) {
        super(context);
        this.context = context;
        init();
    }

    public NativeImageView(final Context context, final AttributeSet attrs) {
        super(context, attrs);
        this.context = context;
        init();
    }

    private void init() {
        nativeAssetManager = new NativeAssetManager(context);

        setZOrderOnTop(true);
        setEGLContextClientVersion(2);
        setEGLConfigChooser(8, 8, 8, 8, 16, 0);

        setRenderer(new MyRenderer());
        getHolder().setFormat(PixelFormat.TRANSLUCENT);

        (new Thread() {
            @Override
            public void run() {
                native_start();
            }
        }).start();
        requestFocus();
        setFocusableInTouchMode(true);

    }

    class MyRenderer implements GLSurfaceView.Renderer {
        @Override
        public void onSurfaceCreated(GL10 gl, EGLConfig c) {
            native_surface_created();
        }

        @Override
        public void onSurfaceChanged(GL10 gl, int w, int h) {
            native_gl_resize(w, h);
        }

        @Override
        public void onDrawFrame(GL10 gl) {
            time = SystemClock.uptimeMillis();

            if (time >= (frameTime + 1000.0f)) {
                frameTime = time;
                avgFPS += framerate;
                framerate = 0;
            }

            if (time >= (fpsTime + 3000.0f)) {
                fpsTime = time;
                avgFPS /= 3.0f;
                Log.d("GLBUFEX", "FPS: " + Float.toString(avgFPS));
                avgFPS = 0;
            }
            framerate++;
            native_gl_render();
            postInvalidate();
        }
        public long time = 0;
        public short framerate = 0;
        public long fpsTime = 0;
        public long frameTime = 0;
        public float avgFPS = 0;
    }

    static {
        System.loadLibrary("nativeimageviewlib");
    }

}
