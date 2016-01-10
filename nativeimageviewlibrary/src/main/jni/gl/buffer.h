//
// Created by Semyon on 10.01.2016.
//

#ifndef NATIVEIMAGEVIEW_BUFFER_H
#define NATIVEIMAGEVIEW_BUFFER_H

#include "platform_gl.h"

#define BUFFER_OFFSET(i) ((void*)(i))

GLuint create_vbo(const GLsizeiptr size, const GLvoid* data, const GLenum usage);

#endif //NATIVEIMAGEVIEW_BUFFER_H
