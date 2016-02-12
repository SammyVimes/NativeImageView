//
// Created by Semyon on 10.01.2016.
//

#include "texture.h"
#include "platform_gl.h"
#include <assert.h>
#include <android/log.h>


#define LOG_TAG "TexturesCPP"
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)

#define UNUSED_ATTR  __attribute__((unused))

static void check_gl_error(const char* op)
{
    GLint error;
    for (error = glGetError(); error; error = glGetError())
        LOGI("after %s() glError (0x%x)\n", op, error);
}

GLuint load_texture(
        const GLsizei width, const GLsizei height,
        const GLenum type, const GLvoid* pixels) {
    GLuint texture_object_id;
    glGenTextures(1, &texture_object_id);
    check_gl_error("glGenTextures");
    assert(texture_object_id != 0);

    glBindTexture(GL_TEXTURE_2D, texture_object_id);
    check_gl_error("glBindTexture");

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    check_gl_error("glTexParameteri 1");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    check_gl_error("glTexParameteri 2");
    glTexImage2D(GL_TEXTURE_2D, 0, type, width, height, 0, type, GL_UNSIGNED_BYTE, pixels);
    LOGI("glTexImage2D %d %d 0x%.8X", width, height, type);
    check_gl_error("glTexImage2D");

    glBindTexture(GL_TEXTURE_2D, 0);
    check_gl_error("glBindTexture");
    return texture_object_id;
}
