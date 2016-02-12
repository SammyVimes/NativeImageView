//
// Created by Semyon on 11.02.2016.
//

#ifndef NATIVEIMAGEVIEW_TEXTUREBUFFER_H
#define NATIVEIMAGEVIEW_TEXTUREBUFFER_H

#include <vector>
#include <GLES/gl.h>

const int TileDim = 128;

template <typename T>
const T & qMax(const T & value1, const T & value2) {
    return value1 > value2 ? value1 : value2;
}

template <typename T>
const T & qMin(const T & value1, const T & value2) {
    return value1 < value2 ? value1 : value2;
}

template <typename T>
const T & qAbs(const T & value) {
    return value > 0 ? value : -value;
}


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

class QRect {

public:

    QRect(int x, int y, int width, int height): _x1(x), _y1(y), _x2(x + width - 1), _y2(y + height - 1) {}

    QRect(QPoint topLeft, QPoint bottomRight): _x1(topLeft.x()), _y1(topLeft.y()), _x2(bottomRight.x()), _y2(bottomRight.y()) {}

    int x() const {
        return _x1;
    }
    int y() const {
        return _y1;
    }
    int right() const {
        return _x2;
    }
    int bottom() const {
        return _y2;
    }

    void adjust(int dx1, int dy1, int dx2, int dy2) {
        _x1 += dx1;
        _y1 += dy1;
        _x2 += dx2;
        _y2 += dy2;
    }

    bool contains(int x, int y) {
        int l, r;
        if (_x2 < _x1 - 1) {
            l = _x2;
            r = _x1;
        } else {
            l = _x1;
            r = _x2;
        }

        if (x < l || x > r) {
            return false;
        }

        int t, b;
        if (_y2 < _y1 - 1) {
            t = _y2;
            b = _y1;
        } else {
            t = _y1;
            b = _y2;
        }
        return !(y < t || y > b);
    }

    bool contains(const QPoint &p)
    {
        return contains(p.x(), p.y());
    }

private:
    int _x1, _y1, _x2, _y2;

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
    QSize getBufferSize() {
        return bufferSize;
    }
    void draw(int x, int y, GLuint substitute = 0) const;
    void setViewModelMatrix(const QPointF &viewOffset, double viewZoomFactor) const;
    QRect visibleRange(const QPointF &viewOffset, double viewZoomFactor,
                       const QSize &viewSize) const;
private:
    std::vector<GLuint> textures;
    QSize bufferSize;
};


// Define a simple 2D vector
typedef struct Vec2 {
    float x,y;
} Vec2;

// Define a simple 4-byte color
typedef struct Color4B {
    GLbyte r,g,b,a;
};

// Define a suitable quad vertex with a color and tex coords.
typedef struct QuadVertex {
    Vec2 vect;              // 8 bytes
    Color4B color;          // 4 bytes
    Vec2 texCoords;         // 8 bytes
} QuadVertex;

typedef struct Quad {
    QuadVertex tl;
    QuadVertex bl;
    QuadVertex tr;
    QuadVertex br;
} Quad;

#endif //NATIVEIMAGEVIEW_TEXTUREBUFFER_H
