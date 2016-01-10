//
// Created by Semyon on 10.01.2016.
//

#ifndef NATIVEIMAGEVIEW_TEXTURE_H
#define NATIVEIMAGEVIEW_TEXTURE_H


#include "platform_gl.h"

GLuint load_texture(
        const GLsizei width, const GLsizei height,
        const GLenum type, const GLvoid* pixels);


#endif //NATIVEIMAGEVIEW_TEXTURE_H
