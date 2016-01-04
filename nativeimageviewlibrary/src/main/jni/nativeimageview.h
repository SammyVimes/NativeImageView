//
// Created by Semyon on 04.01.2016.
//

#ifndef NATIVEIMAGEVIEW_NATIVEIMAGEVIEW_H
#define NATIVEIMAGEVIEW_NATIVEIMAGEVIEW_H

#include <stdlib.h>
#include <jni.h>

#include <string.h>
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <android/log.h>
#include <pthread.h>
#include "png/PNGDecoder.h"
#include "image.h"

#define LOG_TAG "JNINativeImageView"
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)

#define TEXTURE_WIDTH 512
#define TEXTURE_HEIGHT 256

#define UNUSED  __attribute__((unused))

static GLuint s_texture = 0;
static int s_x = 10;
static int s_y = 50;

int s_w = 0;
int s_h = 0;

static void check_gl_error(const char* op)
{
    GLint error;
    for (error = glGetError(); error; error = glGetError())
        LOGI("after %s() glError (0x%x)\n", op, error);
}

/* disable these capabilities. */
static GLuint s_disable_caps[] = {
        GL_FOG,
        GL_LIGHTING,
        GL_CULL_FACE,
        GL_ALPHA_TEST,
        GL_BLEND,
        GL_COLOR_LOGIC_OP,
        GL_DITHER,
        GL_STENCIL_TEST,
        GL_DEPTH_TEST,
        GL_COLOR_MATERIAL,
        0
};

extern "C" JNIEXPORT
void JNICALL Java_com_github_sammyvimes_nativeimageviewlibrary_NativeImageView_native_1start(JNIEnv* UNUSED, jclass UNUSED) {
    LOGI("START");
}

extern "C" JNIEXPORT
void JNICALL Java_com_github_sammyvimes_nativeimageviewlibrary_NativeImageView_native_1gl_1render(JNIEnv* UNUSED, jclass UNUSED) {
    GLuint texture1;
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


    //читаем PNG картинку
    PNGDecoder pngDecoder;
    image im = pngDecoder.readPng("/mnt/sdcard/image.png"); //TODO: here i fall
    LOGI("PNG: %dx%d (%dx%d) bit:%d type:%d", im.imWidth, im.imHeight, im.glWidth, im.glHeight, im.bit_depth, im.color_type);
    //в зависимости от прозрачности загружаем текстуру в OpenGL
    if (im.color_type == PNG_COLOR_TYPE_RGBA) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, im.glWidth, im.glHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, im.data);
    } else {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, im.glWidth, im.glHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, im.data);
    }
    glClear(GL_COLOR_BUFFER_BIT);
    check_gl_error("glTexImage2D");
    glDrawTexiOES(0, 0, 0, s_w, s_h);
    check_gl_error("glDrawTexiOES");
}

extern "C" JNIEXPORT
void JNICALL Java_com_github_sammyvimes_nativeimageviewlibrary_NativeImageView_native_1gl_1resize(JNIEnv* UNUSED, jclass UNUSED, jint w, jint h) {
    LOGI("native_gl_resize %d %d", w, h);
    glDeleteTextures(1, &s_texture);
    GLuint *start = s_disable_caps;
    while (*start) {
        glDisable(*start++);
    }
    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &s_texture);
    glBindTexture(GL_TEXTURE_2D, s_texture);
    glTexParameterf(GL_TEXTURE_2D,
                    GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D,
                    GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glShadeModel(GL_FLAT);
    check_gl_error("glShadeModel");
    glColor4x(0x10000, 0x10000, 0x10000, 0x10000);
    check_gl_error("glColor4x");
    int rect[4] = {0, TEXTURE_HEIGHT, TEXTURE_WIDTH, -TEXTURE_HEIGHT};
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, rect);
    check_gl_error("glTexParameteriv");
    s_w = w;
    s_h = h;
}



#endif //NATIVEIMAGEVIEW_NATIVEIMAGEVIEW_H
