//
// Created by Semyon on 11.02.2016.
//

#include "TextureBuffer.h"
#include <android/log.h>
#include <GLES2/gl2.h>
#include "matrix.h"

#define LOG_TAG "TextureBuffer"
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)


TextureBuffer::TextureBuffer()
        : viewOffset(0.0f, 0.0f),
        zoomFactor(0.1) {
}

TextureBuffer &TextureBuffer::operator=(const TextureBuffer &buffer) {
    textures = buffer.textures;
    zoomFactor = buffer.zoomFactor;
    bufferSize = buffer.bufferSize;
    return *this;
}

bool TextureBuffer::isEmpty() const {
    return textures.empty();
}

void TextureBuffer::clear() {
    if (!textures.empty()) {
        glDeleteTextures(textures.size(), &textures[0]);
        textures.clear();
    }
    bufferSize = QSize(0, 0);
}

GLuint TextureBuffer::at(int x, int y) const {
    int index = y * bufferSize.width() + x;
    if (index < 0 || index >= textures.size())
        return 0;
    return textures.at(index);
}

// Note: does not free the existing texture!
void TextureBuffer::replace(int x, int y, GLuint texture) {
    int index = y * bufferSize.width() + x;
    if (index < 0 || index >= textures.size())
        return;
    textures[index] = texture;
}

void TextureBuffer::remove(int x, int y) {
    int index = y * bufferSize.width() + x;
    if (index < 0 || index >= textures.size())
        return;
    if (textures.at(index) != 0) {
        glDeleteTextures(1, &textures[0] + index);
        textures[index] = 0;
    }
}

int TextureBuffer::width() const {
    return bufferSize.width();
}

int TextureBuffer::height() const {
    return bufferSize.height();
}

void TextureBuffer::resize(int w, int h) {
    bufferSize = QSize(w, h);
    textures.resize(w * h);
    std::fill(textures.begin(), textures.end(), 0);
}

float* ortho = 0;

void matrixIdentity(float* m)
{
    m[0] = m[5] = m[10] = m[15] = 1.0;
    m[1] = m[2] = m[3] = m[4] = 0.0;
    m[6] = m[7] = m[8] = m[9] = 0.0;
    m[11] = m[12] = m[13] = m[14] = 0.0;
}

void matrixTranslate(float x, float y, float z, float* matrix)
{
    matrixIdentity(matrix);
    // Translate slots.
    matrix[12] = x;
    matrix[13] = y;
    matrix[14] = z;
}

void matrixScale(float sx, float sy, float sz, float* matrix)
{
    matrixIdentity(matrix);
    // Scale slots.
    matrix[0] = sx;
    matrix[5] = sy;
    matrix[10] = sz;
}

float* gen_ortho(float left, float right, float top, float bottom, float near, float far){
    float* result = new float[16];
    result[0] = 2.0f / (right - left);
    result[1] = 0.0;
    result[2] = 0.0;
    result[3] = 0.0;
    result[4] = 0.0;
    result[5] = 2.0f / (top - bottom);
    result[6] = 0.0;
    result[7] = 0.0;
    result[8] = 0.0;
    result[9] = 0.0;
    result[10] = (1.0f / (near - far));
    result[11] = 0.0;
    result[12] = -(right + left) / (right - left);
    result[13] = -(top + bottom) / (top - bottom);
    result[14] = -(far + near) / (far - near);
    result[15] = 1;
    return result;
}

void matrixMultiply(float* m1, float* m2, float* result)
{
    // Fisrt Column
    result[0] = m1[0]*m2[0] + m1[4]*m2[1] + m1[8]*m2[2] + m1[12]*m2[3];
    result[1] = m1[1]*m2[0] + m1[5]*m2[1] + m1[9]*m2[2] + m1[13]*m2[3];
    result[2] = m1[2]*m2[0] + m1[6]*m2[1] + m1[10]*m2[2] + m1[14]*m2[3];
    result[3] = m1[3]*m2[0] + m1[7]*m2[1] + m1[11]*m2[2] + m1[15]*m2[3];

    // Second Column
    result[4] = m1[0]*m2[4] + m1[4]*m2[5] + m1[8]*m2[6] + m1[12]*m2[7];
    result[5] = m1[1]*m2[4] + m1[5]*m2[5] + m1[9]*m2[6] + m1[13]*m2[7];
    result[6] = m1[2]*m2[4] + m1[6]*m2[5] + m1[10]*m2[6] + m1[14]*m2[7];
    result[7] = m1[3]*m2[4] + m1[7]*m2[5] + m1[11]*m2[6] + m1[15]*m2[7];

    // Third Column
    result[8] = m1[0]*m2[8] + m1[4]*m2[9] + m1[8]*m2[10] + m1[12]*m2[11];
    result[9] = m1[1]*m2[8] + m1[5]*m2[9] + m1[9]*m2[10] + m1[13]*m2[11];
    result[10] = m1[2]*m2[8] + m1[6]*m2[9] + m1[10]*m2[10] + m1[14]*m2[11];
    result[11] = m1[3]*m2[8] + m1[7]*m2[9] + m1[11]*m2[10] + m1[15]*m2[11];

    // Fourth Column
    result[12] = m1[0]*m2[12] + m1[4]*m2[13] + m1[8]*m2[14] + m1[12]*m2[15];
    result[13] = m1[1]*m2[12] + m1[5]*m2[13] + m1[9]*m2[14] + m1[13]*m2[15];
    result[14] = m1[2]*m2[12] + m1[6]*m2[13] + m1[10]*m2[14] + m1[14]*m2[15];
    result[15] = m1[3]*m2[12] + m1[7]*m2[13] + m1[11]*m2[14] + m1[15]*m2[15];
}

void writeMat(float* mat, const char *name) {
    LOGI("%s mat: {{%f, %f, %f, %f}, {%f, %f, %f, %f}, {%f, %f, %f, %f}, {%f, %f, %f, %f}}", name,
         mat[0], mat[4], mat[8], mat[12],
         mat[1], mat[5], mat[9], mat[13],
         mat[2], mat[6], mat[10], mat[14],
         mat[3], mat[7], mat[11], mat[15]);
}

void TextureBuffer::draw(int x, int y, GLuint substitute, GLint u_texture_unit_location, GLint mvp_matrix_location) const {
    int index = y * bufferSize.width() + x;
    if (index < 0 || index >= textures.size())
        return;

    GLuint texture = textures.at(index);

    LOGI("Tex handle is %d", texture);
    if (texture == 0)
        texture = substitute;
    if (texture == 0)
        return;
    LOGI("Final tex handle is %d", texture);

    float tx = x * TileDim;
    float ty = y * TileDim;

    float near = -1;
    float far = 1;

    if (ortho) {
        delete[] ortho;
        ortho = 0;
    }

    float aR = (float) viewHeight / (float) viewWidth;

    ortho = gen_ortho(-aR, aR, 1, -1, near, far);
    LOGI("X is %d Y is %d", x, y);

    writeMat(ortho, "ortho1");

    float translate[16];
    matrixTranslate((float) (viewOffset.x() + tx), (float) (viewOffset.y() + ty), 0, translate);

    writeMat(translate, "translate");

    float scale[16];
    matrixScale(((float) TileDim) / (float) viewWidth, (float) TileDim / (float) viewHeight, 1, scale);
//    float scale2[16];
//    matrixScaleM(scale2, (float) viewZoomFactor / (float) zoomFactor, (float) viewZoomFactor / (float) zoomFactor, 1);
//
//    float fullScale[16];
//    matrixMultiply(fullScale, scale, scale2);

    writeMat(scale, "scale");

    float model[16];
    matrixMultiply(translate, scale, model);
    
    matrixMultiplyMM(ortho, ortho, model);

    writeMat(ortho, "ortho2");

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(u_texture_unit_location, 0);
    glUniformMatrix4fv(mvp_matrix_location, 1, 0, ortho);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void TextureBuffer::setViewModelMatrix(const QPointF &viewOffset, double viewZoomFactor, int viewWidth, int viewHeight) {
    this->viewOffset = viewOffset;
    this->viewWidth = viewWidth;
    this->viewHeight = viewHeight;
    this->viewZoomFactor = viewZoomFactor;
}

QRect TextureBuffer::visibleRange(const QPointF &viewOffset,
                                  double viewZoomFactor, const QSize &viewSize) const {
    double dim = TileDim * viewZoomFactor / zoomFactor;

    int tx1 = -viewOffset.x() / dim;
    int tx2 = (viewSize.width() - viewOffset.x()) / dim;
    int ty1 = -viewOffset.y() / dim;
    int ty2 = (viewSize.height() - viewOffset.y()) / dim;

    tx1 = qMax(0, tx1);
    tx2 = qMin(bufferSize.width() - 1, tx2);
    ty1 = qMax(0, ty1);
    ty2 = qMin(bufferSize.height() - 1, ty2);

    return QRect(QPoint(tx1, ty1), QPoint(tx2, ty2));
}