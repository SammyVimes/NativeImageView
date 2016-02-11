//
// Created by Semyon on 11.02.2016.
//

#ifndef NATIVEIMAGEVIEW_TEXTUREBUFFER_H
#define NATIVEIMAGEVIEW_TEXTUREBUFFER_H

#include <vector>
#include <GLES/gl.h>

template <typename T>
const T & qMax(const T & value1, const T & value2) {
    return value1 > value2 ? value1 : value2;
}

template <typename T>
const T & qMin(const T & value1, const T & value2) {
    return value1 < value2 ? value1 : value2;
}

class QRect {

public:

    QRect(int x, int y, int width, int height) {
        this->_x = x;
        this->_y = y;
        this->_width = width;
        this->_height = height;
    }

    QRect(QPoint topLeft, QPoint bottomRight) {
        this->_x = topLeft.x();
        this->_y = topLeft.y();
        this->_width = bottomRight.x() - this->_x;
        this->_height = this->_y - bottomRight.y();
    }
    int x() const {
        return _x;
    }
    int y() const {
        return _y;
    }
    int width() const {
        return _width;
    }
    int height() const {
        return _height;
    }

private:
    int _x, _y, _width, _height;

};

class QPointF {
public:
    QPointF(double x, double y) {
        this->_x = x;
        this->_y = y;
    }

    double x() const {
        return _x;
    }

    double y() const {
        return _y;
    }
private:
    double _x, _y;
};

class QPoint {
public:
    QPoint(int _x, int _y) {
        this->_x = _x;
        this->_y = _y;
    }

    int x() const {
        return _x;
    }

    int y() const {
        return _y;
    }

private:
    int _x, _y;
};

class QSize {

public:
    QSize() {
        this->_width = 0;
        this->_height = 0;
    }

    QSize(int width, int height) {
        this->_height = height;
        this->_width = width;
    }
    int width() const {
        return _width;
    }

    int height() const {
        return _height;
    }

private:
    int _width, _height;

};

class TextureBuffer {

public:
    double zoomFactor;

    TextureBuffer();
    TextureBuffer& operator=(const TextureBuffer&);

    bool isEmpty() const;
    void clear();

    GLuint at(int x, int y) const;
    void replace(int x, int y, GLuint texture);
    void remove(int x, int y);

    int width() const;
    int height() const;
    void resize(int w, int h);

    void draw(int x, int y, GLuint substitute = 0) const;
    void setViewModelMatrix(const QPointF &viewOffset, double viewZoomFactor) const;
    QRect visibleRange(const QPointF &viewOffset, double viewZoomFactor,
                       const QSize &viewSize) const;
private:
    std::vector<GLuint> textures;
    QSize bufferSize;

};


#endif //NATIVEIMAGEVIEW_TEXTUREBUFFER_H
