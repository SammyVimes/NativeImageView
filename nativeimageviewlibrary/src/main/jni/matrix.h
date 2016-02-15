//
// Created by Semyon on 15.02.2016.
//

#ifndef NATIVEIMAGEVIEW_MATRIX_H
#define NATIVEIMAGEVIEW_MATRIX_H

#include <stdlib.h>
#include <math.h>

#define PI 3.1415926f
#define normalize(x, y, z)                  \
{                                               \
        float norm = 1.0f / sqrt(x*x+y*y+z*z);  \
        x *= norm; y *= norm; z *= norm;        \
}
#define I(_i, _j) ((_j)+4*(_i))


void matrixSetRotateM(float *m, float a, float x, float y, float z);

void matrixMultiplyMM(float *m, float *lhs, float *rhs);

void matrixScaleM(float *m, float x, float y, float z);

void matrixTranslateM(float *m, float x, float y, float z);

void matrixRotateM(float *m, float a, float x, float y, float z);

void matrixLookAtM(float *m,
                   float eyeX, float eyeY, float eyeZ,
                   float cenX, float cenY, float cenZ,
                   float  upX, float  upY, float  upZ);

void matrixFrustumM(float *m, float left, float right, float bottom, float top, float near, float far);

#endif //NATIVEIMAGEVIEW_MATRIX_H
