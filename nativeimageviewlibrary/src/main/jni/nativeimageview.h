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
#include "gl/texture.h"
#include "gl/shader.h"
#include "gl/buffer.h"
#include "file/platform_file_utils.h"

#define LOG_TAG "JNINativeImageView"
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)

#define TEXTURE_WIDTH 512
#define TEXTURE_HEIGHT 256

#define UNUSED_ATTR  __attribute__((unused))


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

class PNGDecoder;


GLuint load_png_asset_into_texture(const char* relative_path) {
    assert(relative_path != NULL);

    const FileData png_file = get_asset_data(relative_path);
    PNGDecoder pngDecoder;
    const RawImageData raw_image_data = pngDecoder.get_raw_image_data_from_png_file("/storage/sdcard1/Pictures/image.png");
    const GLuint texture_object_id = load_texture(
            raw_image_data.width, raw_image_data.height,
            raw_image_data.gl_color_format, raw_image_data.data);

    pngDecoder.release_raw_image_data(&raw_image_data);
    release_asset_data(&png_file);

    return texture_object_id;
}

GLuint load_png_file_into_texture(const char* relative_path) {
    LOGI("load_png_file_into_texture");
    assert(relative_path != NULL);

    PNGDecoder pngDecoder;
    const RawImageData raw_image_data = pngDecoder.get_raw_image_data_from_png_file(relative_path);
    const GLuint texture_object_id = load_texture(
            raw_image_data.width, raw_image_data.height,
            raw_image_data.gl_color_format, raw_image_data.data);

    pngDecoder.release_raw_image_data(&raw_image_data);

    return texture_object_id;
}

GLuint build_program_from_assets(
        const char* vertex_shader_path, const char* fragment_shader_path) {
    assert(vertex_shader_path != NULL);
    assert(fragment_shader_path != NULL);

    const FileData vertex_shader_source = get_asset_data(vertex_shader_path);
    const FileData fragment_shader_source = get_asset_data(fragment_shader_path);
    const GLuint program_object_id = build_program(
            (const GLchar *) vertex_shader_source.data, vertex_shader_source.data_length,
            (const GLchar *) fragment_shader_source.data, fragment_shader_source.data_length);

    release_asset_data(&vertex_shader_source);
    release_asset_data(&fragment_shader_source);

    return program_object_id;
}


static GLuint texture;
static GLuint buffer;
static GLuint program;

static GLint a_position_location;
static GLint a_texture_coordinates_location;
static GLint u_texture_unit_location;

// position X, Y, texture S, T
static const float rect[] = {-1.0f, 1.0f, 0.0f, 0.0f,
                             -1.0f,  -1.0f, 0.0f, 1.0f,
                             1.0f, 1.0f, 1.0f, 0.0f,
                             1.0f,  -1.0f, 1.0f, 1.0f};

extern "C" JNIEXPORT
void JNICALL Java_com_github_sammyvimes_nativeimageviewlibrary_NativeImageView_native_1start(JNIEnv* UNUSED_ATTR, jclass UNUSED_ATTR) {
    LOGI("START");
}

extern "C" JNIEXPORT
void JNICALL Java_com_github_sammyvimes_nativeimageviewlibrary_NativeImageView_native_1surface_1created(JNIEnv* UNUSED_ATTR, jclass UNUSED_ATTR) {
    LOGI("SURFACE CREATED");
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
}

extern "C" JNIEXPORT
void JNICALL Java_com_github_sammyvimes_nativeimageviewlibrary_NativeImageView_native_1gl_1render(JNIEnv* UNUSED_ATTR, jclass UNUSED_ATTR) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(program);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(u_texture_unit_location, 0);

    GLint valPerVerticeCoordSize = 2;
    GLint verticeOffset = 0;

    GLint valPerTextureCoordSize = 2;
    GLint coordOffset = 2;

    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glVertexAttribPointer(a_position_location, valPerVerticeCoordSize, GL_FLOAT, GL_FALSE,
                          4 * sizeof(GL_FLOAT), BUFFER_OFFSET(verticeOffset));
    glVertexAttribPointer(a_texture_coordinates_location, valPerTextureCoordSize, GL_FLOAT, GL_FALSE,
                          4 * sizeof(GL_FLOAT), BUFFER_OFFSET(coordOffset * sizeof(GL_FLOAT)));
    glEnableVertexAttribArray(a_position_location);
    glEnableVertexAttribArray(a_texture_coordinates_location);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

extern "C" JNIEXPORT
void JNICALL Java_com_github_sammyvimes_nativeimageviewlibrary_NativeImageView_native_1gl_1resize(JNIEnv* UNUSED_ATTR, jclass UNUSED_ATTR, jint w, jint h) {
    LOGI("native_gl_resize %d %d", w, h);
//    glDeleteTextures(1, &s_texture);
//    GLuint *start = s_disable_caps;
//    while (*start) {
//        glDisable(*start++);
//    }
//    glEnable(GL_TEXTURE_2D);
//    glGenTextures(1, &s_texture);
//    glBindTexture(GL_TEXTURE_2D, s_texture);
//    glTexParameterf(GL_TEXTURE_2D,
//                    GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glTexParameterf(GL_TEXTURE_2D,
//                    GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    glShadeModel(GL_FLAT);
//    check_gl_error("glShadeModel");
//    glColor4x(0x10000, 0x10000, 0x10000, 0x10000);
//    check_gl_error("glColor4x");
//    int rect[4] = {0, TEXTURE_HEIGHT, TEXTURE_WIDTH, -TEXTURE_HEIGHT};
//    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, rect);
//    check_gl_error("glTexParameteriv");
//    s_w = w;
//    s_h = h;
    texture = load_png_file_into_texture("/storage/sdcard1/Pictures/image.png");
    buffer = create_vbo(sizeof(rect), rect, GL_STATIC_DRAW);
    program = build_program_from_assets("shaders/shader.vsh", "shaders/shader.fsh");

    a_position_location = glGetAttribLocation(program, "a_Position");
    a_texture_coordinates_location =
            glGetAttribLocation(program, "a_TextureCoordinates");
    u_texture_unit_location = glGetUniformLocation(program, "u_TextureUnit");
}


#endif //NATIVEIMAGEVIEW_NATIVEIMAGEVIEW_H
