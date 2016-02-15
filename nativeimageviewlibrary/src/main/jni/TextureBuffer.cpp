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

void TextureBuffer::draw(int x, int y, GLuint substitute, GLint u_texture_unit_location, GLint mvp_matrix_location) const {
    int index = y * bufferSize.width() + x;
    if (index < 0 || index >= textures.size())
        return;

    GLuint texture = textures.at(index);
    //FIXME: tex handle is 0!!
    LOGI("Tex handle is %d", texture);
    if (texture == 0)
        texture = substitute;
    if (texture == 0)
        return;
    LOGI("Final tex handle is %d", texture);

    float tx = x * TileDim;
    float ty = y * TileDim;


    float left = 0;
    float right = viewWidth;
    float bottom = 0;
    float top = viewHeight;
    float near = -1;
    float far = -1;
    float a = 2.0f / (right - left);
    float b = 2.0f / (top - bottom);
    float c = -2.0f / (far - near);

    float ox = - (right + left)/(right - left);
    float oy = - (top + bottom)/(top - bottom);
    float oz = - (far + near)/(far - near);


    float ortho[16] = {
            a, 0, 0, 0,
            0, b, 0, 0,
            0, 0, c, 0,
            ox, oy, oz, 1
    };

    matrixTranslateM(ortho, viewOffset.x(), viewOffset.y(), 0);
    matrixTranslateM(ortho, tx, ty, 0);
    matrixScaleM(ortho, TileDim / viewWidth, TileDim / viewHeight, 1);
    matrixScaleM(ortho, viewZoomFactor / zoomFactor, viewZoomFactor / zoomFactor, 1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(u_texture_unit_location, 0);
    glUniformMatrix4fv(mvp_matrix_location, 1, 0, &ortho[0]);

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