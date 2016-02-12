//
// Created by Semyon on 11.02.2016.
//

#include "TextureBuffer.h"

TextureBuffer::TextureBuffer()
        : zoomFactor(0.1) {
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

// Each tile is drawn as textured quad.
void TextureBuffer::draw(int x, int y, GLuint substitute) const {
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

    Quad quad;
    quad.bl.vect = (Vec2) {tx, ty};
    quad.br.vect = (Vec2) {tx + TileDim, ty};
    quad.tr.vect = (Vec2) {tx + TileDim, ty + TileDim};
    quad.tl.vect = (Vec2) {tx, ty + TileDim};
    quad.tl.color = quad.tr.color = quad.bl.color = quad.br.color
            = (Color4B) {0, 0, 0, 255};
    quad.tl.texCoords = (Vec2) {0, 0};
    quad.tr.texCoords = (Vec2) {1, 0};
    quad.br.texCoords = (Vec2) {1, 1};
    quad.bl.texCoords = (Vec2) {0, 1};

    // "Explain" the quad structure to OpenGL ES

    #define kQuadSize sizeof(quad.bl)
    long offset = (long) &quad;

    // vertex
    int diff = offsetof(QuadVertex, vect);
    glVertexPointer(2, GL_FLOAT, kQuadSize, (void *) (offset + diff));

    // color
    diff = offsetof(QuadVertex, color);
    glColorPointer(4, GL_UNSIGNED_BYTE, kQuadSize, (void *) (offset + diff));

    // texCoods
    diff = offsetof(QuadVertex, texCoords);
    glTexCoordPointer(2, GL_FLOAT, kQuadSize, (void *) (offset + diff));

    glBindTexture(GL_TEXTURE_2D, texture);

    // Draw the quad
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void TextureBuffer::setViewModelMatrix(const QPointF &viewOffset, double viewZoomFactor) const {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(viewOffset.x(), viewOffset.y(), 0);
    glScalef(viewZoomFactor / zoomFactor, viewZoomFactor / zoomFactor, 1);
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