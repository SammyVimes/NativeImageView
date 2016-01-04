//
// Created by Semyon on 04.01.2016.
//

#ifndef NATIVEIMAGEVIEW_PNGDECODER_H
#define NATIVEIMAGEVIEW_PNGDECODER_H

#include "stdio.h"
#include <android/log.h>
#include <setjmp.h>
#include <zconf.h>

#include "libpng/png.h"
#include "../image.h"

#define LOG(...) __android_log_print(ANDROID_LOG_VERBOSE, "NDK",__VA_ARGS__)


class PNGDecoder {

public:
    image readPng(const char* fileName) {
        image im;
        FILE* file = fopen(fileName, "rb");
        //пропускаем заголовок, хотя именно сюда можно добавить проверку PNG это или JPEG, чтобы ф-ция сама определяла как грузить картинку
        fseek(file, 8, SEEK_CUR);

        png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        png_infop info_ptr = png_create_info_struct(png_ptr);

        png_init_io(png_ptr, file);
        png_set_sig_bytes(png_ptr, 8);
        png_read_info(png_ptr, info_ptr);

        //читаем данные о картинке
        png_get_IHDR(png_ptr, info_ptr, &im.imWidth, &im.imHeight, &im.bit_depth, &im.color_type, NULL, NULL, NULL);

        //определяем размер картинки подходящий для OpenGL
        im.glWidth = reNpot(im.imWidth);
        im.glHeight = reNpot(im.imHeight);

        //если картинка содержит прозрачность то на каждый пиксель 4 байта (RGBA), иначе 3 (RGB)
        int row = im.glWidth * (im.color_type == PNG_COLOR_TYPE_RGBA ? 4 : 3);
        im.data = new char[row * im.glHeight];

        //в этом массиве содержатся указатели на начало каждой строки
        png_bytep * row_pointers = new png_bytep[im.imHeight];
        for(int i = 0; i < im.imHeight; ++i)
            row_pointers[i] = (png_bytep) (im.data + i * row);

        //читаем картинку
        png_read_image(png_ptr, row_pointers);
        png_destroy_read_struct(&png_ptr, &info_ptr, 0);
        delete[] row_pointers;

        return im;
    }

};


#endif //NATIVEIMAGEVIEW_PNGDECODER_H
