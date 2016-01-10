//
// Created by Semyon on 04.01.2016.
//

#ifndef NATIVEIMAGEVIEW_IMAGE_H
#define NATIVEIMAGEVIEW_IMAGE_H

#include "png/libpng/png.h"
#include "gl/platform_gl.h"

struct image {
    png_uint_32 imWidth, imHeight; //реальный размер картинки
    png_uint_32 glWidth, glHeight; //размер который подойдет для OpenGL
    int bit_depth, color_type;
    char* data; //данные RGB/RGBA
};

typedef struct {
    const int width;
    const int height;
    const int size;
    const GLenum gl_color_format;
    const void* data;
} RawImageData;



static int reNpot(int w) {
    //поддерживает ли OpenGL текстуры размера не кратным двум
    //эту переменную конечно надо определять один раз при старте проги с помощью
    //String s = gl.glGetString(GL10.GL_EXTENSIONS);
    //NON_POWER_OF_TWO_SUPPORTED = s.contains("texture_2D_limited_npot") || s.contains("texture_npot") || s.contains("texture_non_power_of_two");
    bool NON_POWER_OF_TWO_SUPPORTED = false;
    if (NON_POWER_OF_TWO_SUPPORTED) {
        if (w % 2) w++;
    } else {
        if (w <= 4) w = 4;
        else if (w <= 8) w = 8;
        else if (w <= 16) w = 16;
        else if (w <= 32) w = 32;
        else if (w <= 64) w = 64;
        else if (w <= 128) w = 128;
        else if (w <= 256) w = 256;
        else if (w <= 512) w = 512;
        else if (w <= 1024) w = 1024;
        else if (w <= 2048) w = 2048;
        else if (w <= 4096) w = 4096;
    }
    return w;
}

#endif //NATIVEIMAGEVIEW_IMAGE_H
