//
// Created by Semyon on 04.01.2016.
//

#ifndef NATIVEIMAGEVIEW_NATIVEIMAGEVIEW_H
#define NATIVEIMAGEVIEW_NATIVEIMAGEVIEW_H

#include <stdlib.h>
#include <jni.h>

#include <string.h>
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <android/log.h>
#include <pthread.h>
#include "png/PNGDecoder.h"
#include "image.h"
#include "gl/texture.h"
#include "gl/shader.h"
#include "gl/buffer.h"
#include "file/platform_file_utils.h"

#include "TextureBuffer.h"
#include <list>
#include <chrono>
#include <functional>
#include <future>
#include <cstdio>
#include <thread>

#define LOG_TAG "JNINativeImageView"
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)

#define TEXTURE_WIDTH 512
#define TEXTURE_HEIGHT 256

#define UNUSED_ATTR  __attribute__((unused))


static GLuint s_texture = 0;
static int s_x = 10;
static int s_y = 50;

int s_w = 0;
int s_h = 0;

static void check_gl_error(const char* op)
{
    GLint error;
    for (error = glGetError(); error; error = glGetError())
        LOGI("after %s() glError (0x%x)\n", op, error);
}

/* disable these capabilities. */
static GLuint s_disable_caps[] = {
        GL_FOG,
        GL_LIGHTING,
        GL_CULL_FACE,
        GL_ALPHA_TEST,
        GL_BLEND,
        GL_COLOR_LOGIC_OP,
        GL_DITHER,
        GL_STENCIL_TEST,
        GL_DEPTH_TEST,
        GL_COLOR_MATERIAL,
        0
};

class PNGDecoder;


GLuint load_png_asset_into_texture(const char* relative_path) {
    assert(relative_path != NULL);

    const FileData png_file = get_asset_data(relative_path);
    PNGDecoder pngDecoder;
    const RawImageData raw_image_data = pngDecoder.get_raw_image_data_from_png_file("/storage/sdcard1/Pictures/image.png");
    const GLuint texture_object_id = load_texture(
            raw_image_data.width, raw_image_data.height,
            raw_image_data.gl_color_format, raw_image_data.data);

    pngDecoder.release_raw_image_data(&raw_image_data);
    release_asset_data(&png_file);

    return texture_object_id;
}

GLuint load_png_file_into_texture(const char* relative_path) {
    LOGI("load_png_file_into_texture");
    assert(relative_path != NULL);

    PNGDecoder pngDecoder;
    const RawImageData raw_image_data = pngDecoder.get_raw_image_data_from_png_file(relative_path);
    const GLuint texture_object_id = load_texture(
            raw_image_data.width, raw_image_data.height,
            raw_image_data.gl_color_format, raw_image_data.data);

    pngDecoder.release_raw_image_data(&raw_image_data);

    return texture_object_id;
}

GLuint build_program_from_assets(
        const char* vertex_shader_path, const char* fragment_shader_path) {
    assert(vertex_shader_path != NULL);
    assert(fragment_shader_path != NULL);

    const FileData vertex_shader_source = get_asset_data(vertex_shader_path);
    const FileData fragment_shader_source = get_asset_data(fragment_shader_path);
    const GLuint program_object_id = build_program(
            (const GLchar *) vertex_shader_source.data, vertex_shader_source.data_length,
            (const GLchar *) fragment_shader_source.data, fragment_shader_source.data_length);

    release_asset_data(&vertex_shader_source);
    release_asset_data(&fragment_shader_source);

    return program_object_id;
}


static GLuint texture;
static GLuint buffer;
static GLuint program;

static GLint a_position_location;
static GLint a_texture_coordinates_location;
static GLint u_texture_unit_location;
static GLint u_texture_matrix_location;

// position X, Y, texture S, T
static const float rect[] = {-1.0f, 1.0f, 0.0f, 0.0f,
                             -1.0f,  -1.0f, 0.0f, 1.0f,
                             1.0f, 1.0f, 1.0f, 0.0f,
                             1.0f,  -1.0f, 1.0f, 1.0f};



class TimerTask
{
public:
    template <class callable, class... arguments>
    TimerTask(int after, bool async, callable&& f, arguments&&... args)
    {
        cancelled = false;
        std::function<typename std::result_of<callable(arguments...)>::type()> task(std::bind(std::forward<callable>(f), std::forward<arguments>(args)...));

        if (async)
        {
            std::thread([this, after, task]() {
                if (this->cancelled) {
                    return;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(after));
                if (this->cancelled) {
                    return;
                }
                task();
            }).detach();
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(after));
            task();
        }
    }

    void cancel() {
        this->cancelled = true;
    }

private:
    bool cancelled;
};

QPoint m_mousePressPosition(0, 0);

TimerTask* updateTask = 0;
TimerTask* refreshTask = 0;

QPointF m_viewOffset(0, 0);
double m_viewZoomFactor = 1;

GLuint m_defaultTexture = 0;
TextureBuffer m_mainBuffer;
TextureBuffer m_secondaryBuffer;
const int ExtraTiles = 2;
const int UpdateDelay = 100;
const int RefreshDelay = 600;

void scheduleUpdate();
void scheduleRefresh();

int _width = 0;
int _height = 0;

int width() {
    return _width;
}

int height() {
    return _height;
}

QSize size() {
    return QSize(_width, _height);
}

void refreshBackingStore()
{
    LOGI("refreshBackingStore");
    if (m_mainBuffer.zoomFactor == m_viewZoomFactor)
        return;

    // All secondary textures serve as the "background overlay".
    m_secondaryBuffer.clear();
    m_secondaryBuffer = m_mainBuffer;

    // Replace the primary textures with an invalid, dirty texture.
//    double width = m_svg.defaultSize().width() * m_viewZoomFactor;
//    double height = m_svg.defaultSize().height() * m_viewZoomFactor;
    double width = 2000;
    double height = 2000;
    int horizontal = (width + TileDim - 1) / TileDim;
    int vertical = (height + TileDim - 1) / TileDim;
    m_mainBuffer.resize(horizontal, vertical);

    m_mainBuffer.zoomFactor = m_viewZoomFactor;
    scheduleUpdate();

    if (refreshTask) {
        refreshTask->cancel();
        refreshTask = 0;
    }
}


void updateBackingStore()
{
    LOGI("updateBackingStore");
    // During zooming in and out, do not bother.
    if (m_mainBuffer.zoomFactor != m_viewZoomFactor)
        return;

    // Extend the update range with extra tiles in every direction, this is
    // to anticipate panning and scrolling.
    QRect updateRange = m_mainBuffer.visibleRange(m_viewOffset, m_viewZoomFactor, size());
    updateRange.adjust(-ExtraTiles, -ExtraTiles, ExtraTiles, ExtraTiles);

    // Collect all visible tiles which need update.
    std::vector<QPoint> dirtyTiles;
    for (int x = 0; x < m_mainBuffer.width(); ++x) {
        for (int y = 0; y < m_mainBuffer.height(); ++y) {
            if (m_mainBuffer.at(x, y) == 0 && updateRange.contains(x, y))
                dirtyTiles.push_back(QPoint(x, y));
        }
    }

    if (!dirtyTiles.empty()) {

        // Find the closest tile to the center (using Manhattan distance)
        int updateX = dirtyTiles.at(0).x();
        int updateY = dirtyTiles.at(0).x();
        double closestDistance = 1e6;
        for (int i = 0; i < dirtyTiles.size(); ++i) {
            int tx = dirtyTiles.at(i).x();
            int ty = dirtyTiles.at(i).y();
            double dim = TileDim * m_viewZoomFactor / m_mainBuffer.zoomFactor;
            double cx = m_viewOffset.x() + dim * (0.5 + tx);
            double cy = m_viewOffset.y() + dim * (0.5 + ty);
            double dist = qAbs(cx - width() / 2) + qAbs(cy - height() / 2);
            if (dist < closestDistance) {
                updateX = tx;
                updateY = ty;
                closestDistance = dist;
            }
        }


        GLuint content = load_png_file_into_texture("/storage/sdcard1/Pictures/tile.png");
        LOGI("NEW TEXTURE LOADED! %d", content);
        m_mainBuffer.replace(updateX, updateY, content);
//        update();
    }


    if (updateTask) {
        updateTask->cancel();
        updateTask = 0;
    }
}


void scheduleUpdate()
{
    if (updateTask) {
        updateTask->cancel();
        updateTask = 0;
    }
    updateTask = new TimerTask(UpdateDelay, true, &updateBackingStore);
}

void scheduleRefresh()
{
    if (refreshTask) {
        refreshTask->cancel();
        refreshTask = 0;
    }
    refreshTask = new TimerTask(RefreshDelay, true, &refreshBackingStore);
}


extern "C" JNIEXPORT
void JNICALL Java_com_github_sammyvimes_nativeimageviewlibrary_NativeImageView_native_1start(JNIEnv* UNUSED_ATTR, jclass UNUSED_ATTR) {
    LOGI("START");
}

extern "C" JNIEXPORT
void JNICALL Java_com_github_sammyvimes_nativeimageviewlibrary_NativeImageView_native_1surface_1created(JNIEnv* UNUSED_ATTR, jclass UNUSED_ATTR) {
    LOGI("SURFACE CREATED");
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    m_defaultTexture = load_png_file_into_texture("/storage/sdcard1/Pictures/checkers.png");
    refreshBackingStore();
}

extern "C" JNIEXPORT
void JNICALL Java_com_github_sammyvimes_nativeimageviewlibrary_NativeImageView_native_1gl_1render(JNIEnv* UNUSED_ATTR, jclass UNUSED_ATTR) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    check_gl_error("glClear");

    glUseProgram(program);
    check_gl_error("glUseProgram");

    GLint valPerVerticeCoordSize = 2;
    GLint verticeOffset = 0;

    GLint valPerTextureCoordSize = 2;
    GLint coordOffset = 2;

    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    check_gl_error("glBindBuffer");
    glVertexAttribPointer(a_position_location, valPerVerticeCoordSize, GL_FLOAT, GL_FALSE,
                          4 * sizeof(GL_FLOAT), BUFFER_OFFSET(verticeOffset));
    check_gl_error("glVertexAttribPointer1");
    glVertexAttribPointer(a_texture_coordinates_location, valPerTextureCoordSize, GL_FLOAT, GL_FALSE,
                          4 * sizeof(GL_FLOAT), BUFFER_OFFSET(coordOffset * sizeof(GL_FLOAT)));
    check_gl_error("glVertexAttribPointer2");
    glEnableVertexAttribArray(a_position_location);
    check_gl_error("glEnableVertexAttribArray1");
    glEnableVertexAttribArray(a_texture_coordinates_location);
    check_gl_error("glEnableVertexAttribArray2");


    // For very fast zooming in and out, do not apply any expensive filter.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // Ensure we would have seamless transition between adjecent tiles.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    check_gl_error("glTexParameteri");

    // This is background, i.e. all the outdated tiles while
    // new primary ones are being prepared (e.g. after zooming).
    if (!m_secondaryBuffer.isEmpty()) {
        QRect backgroundRange = m_secondaryBuffer.visibleRange(m_viewOffset, m_viewZoomFactor, size());
        m_secondaryBuffer.setViewModelMatrix(m_viewOffset, m_viewZoomFactor, _width, _height);

        for (int x = 0; x < m_secondaryBuffer.width(); ++x) {
            for (int y = 0; y < m_secondaryBuffer.height(); ++y) {
                if (backgroundRange.contains(x, y)) {
                    m_secondaryBuffer.draw(x, y, m_defaultTexture, u_texture_unit_location, u_texture_matrix_location);
                    check_gl_error("secondary buffer draw");
                } else {
                    m_secondaryBuffer.remove(x, y);
                }
            }
        }
    }

    // Extend the update range with extra tiles in every direction, this is
    // to anticipate panning and scrolling.
    QRect updateRange = m_mainBuffer.visibleRange(m_viewOffset, m_viewZoomFactor, size());
    LOGI("m_viewZoomFactor %f", m_viewZoomFactor);
    QSize bufferSize = m_mainBuffer.getBufferSize();
    LOGI("bufferSize %d %d", bufferSize.width(), bufferSize.height());
    QSize sz = size();
    LOGI("sz %d %d", sz.width(), sz.height());
    LOGI("UpdateRange %d %d %d %d", updateRange.x(), updateRange.y(), updateRange.right(), updateRange.bottom());
    updateRange.adjust(-ExtraTiles, -ExtraTiles, ExtraTiles, ExtraTiles);

    // When zooming in/out, we have secondary textures as
    // the background. Thus, do not overdraw the background
    // with the checkerboard pattern (default texture).
    GLuint substitute = m_secondaryBuffer.isEmpty() ? m_defaultTexture : 0;

    LOGI("RENDERING");

    m_mainBuffer.setViewModelMatrix(m_viewOffset, m_viewZoomFactor, _width, _height);
    check_gl_error("setViewModelMatrix");
    bool needsUpdate = false;
    LOGI("Main Buffer width %d height %d", m_mainBuffer.width(), m_mainBuffer.height());
    for (int x = 0; x < m_mainBuffer.width(); ++x) {
        for (int y = 0; y < m_mainBuffer.height(); ++y) {
            GLuint texture = m_mainBuffer.at(x, y);
//            LOGI("Texture is %d", texture);
            if (updateRange.contains(x, y)) {
                LOGI("Range contains");
                LOGI("Substitute %d", substitute);
                m_mainBuffer.draw(x, y, substitute, u_texture_unit_location, u_texture_matrix_location);
                check_gl_error("main buffer draw");
                if (texture == 0)
                    needsUpdate = true;
            }

            // Save GPU memory and throw out unneeded texture
            if (texture != 0 && !updateRange.contains(x, y))
                m_mainBuffer.remove(x, y);
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if (needsUpdate) {
        scheduleUpdate();
    } else {
        // Every tile is up-to-date, thus discard the background.
        if (!m_secondaryBuffer.isEmpty()) {
            m_secondaryBuffer.clear();
//            update();
        }
    }

    // Zooming means we need a fresh set of resolution-correct tiles.
    if (m_viewZoomFactor != m_mainBuffer.zoomFactor) {
        scheduleRefresh();
    }
}

extern "C" JNIEXPORT
void JNICALL Java_com_github_sammyvimes_nativeimageviewlibrary_NativeImageView_native_1gl_1resize(JNIEnv* UNUSED_ATTR, jclass UNUSED_ATTR, jint w, jint h) {
    LOGI("native_gl_resize %d %d", w, h);
    _width = w;
    _height = h;
    glViewport(0, 0, w, h);

    texture = load_png_file_into_texture("/storage/sdcard1/Pictures/image.png");
    buffer = create_vbo(sizeof(rect), rect, GL_STATIC_DRAW);
    program = build_program_from_assets("shaders/shader.vsh", "shaders/shader.fsh");

    a_position_location = glGetAttribLocation(program, "a_Position");
    a_texture_coordinates_location =
            glGetAttribLocation(program, "a_TextureCoordinates");
    u_texture_unit_location = glGetUniformLocation(program, "u_TextureUnit");
    u_texture_matrix_location = glGetUniformLocation(program, "u_TexMVPMatrix");
}




#endif //NATIVEIMAGEVIEW_NATIVEIMAGEVIEW_H
