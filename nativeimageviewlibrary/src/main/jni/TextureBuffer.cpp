//
// Created by Semyon on 11.02.2016.
//

#include "TextureBuffer.h"

TextureBuffer::TextureBuffer()
        : zoomFactor(0.1)
{
}

TextureBuffer& TextureBuffer::operator =(const TextureBuffer &buffer)
{
    textures = buffer.textures;
    zoomFactor = buffer.zoomFactor;
    bufferSize = buffer.bufferSize;
    return *this;
}

bool TextureBuffer::isEmpty() const
{
    return textures.empty();
}

void TextureBuffer::clear()
{
    if (!textures.empty()) {
        glDeleteTextures(textures.size(), &textures[0]);
        textures.clear();
    }
    bufferSize = QSize(0, 0);
}

GLuint TextureBuffer::at(int x, int y) const
{
    int index = y * bufferSize.width() + x;
    if (index < 0 || index >= textures.size())
        return 0;
    return textures.at(index);
}

// Note: does not free the existing texture!
void TextureBuffer::replace(int x, int y, GLuint texture)
{
    int index = y * bufferSize.width() + x;
    if (index < 0 || index >= textures.size())
        return;
    textures[index] = texture;
}

void TextureBuffer::remove(int x, int y)
{
    int index = y * bufferSize.width() + x;
    if (index < 0 || index >= textures.size())
        return;
    if (textures.at(index) != 0) {
        glDeleteTextures(1, &textures[0] + index);
        textures[index] = 0;
    }
}

int TextureBuffer::width() const
{
    return bufferSize.width();
}

int TextureBuffer::height() const
{
    return bufferSize.height();
}

void TextureBuffer::resize(int w, int h)
{
    bufferSize = QSize(w, h);
    textures.resize(w * h);
    std::fill(textures.begin(), textures.end(), 0);
}

// Each tile is drawn as textured quad.
void TextureBuffer::draw(int x, int y, GLuint substitute) const
{
    int index = y * bufferSize.width() + x;
    if (index < 0 || index >= textures.size())
        return;

    GLuint texture = textures.at(index);
    if (texture == 0)
        texture = substitute;
    if (texture == 0)
        return;

    float tx = x * TileDim;
    float ty = y * TileDim;

    glBindTexture(GL_TEXTURE_2D, texture);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(tx, ty);
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(tx + TileDim, ty);
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(tx + TileDim, ty + TileDim);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(tx, ty + TileDim);
    glEnd();
}

void TextureBuffer::setViewModelMatrix(const QPointF &viewOffset, double viewZoomFactor) const
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(viewOffset.x(), viewOffset.y(), 0);
    glScalef(viewZoomFactor / zoomFactor, viewZoomFactor / zoomFactor, 1);
}

QRect TextureBuffer::visibleRange(const QPointF &viewOffset,
                                  double viewZoomFactor, const QSize &viewSize) const
{
    double dim = TileDim * viewZoomFactor / zoomFactor;

    int tx1 = -viewOffset.x() / dim;
    int tx2 = (viewSize.width() -viewOffset.x()) / dim;
    int ty1 = -viewOffset.y() / dim;
    int ty2 = (viewSize.height() - viewOffset.y()) / dim;

    tx1 = qMax(0, tx1);
    tx2 = qMin(bufferSize.width() - 1, tx2);
    ty1 = qMax(0, ty1);
    ty2 = qMin(bufferSize.height() - 1, ty2);

    return QRect(QPoint(tx1, ty1), QPoint(tx2, ty2));
}