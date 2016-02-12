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

static void check_gl_error(const char* op)
{
    GLint error;
    for (error = glGetError(); error; error = glGetError())
        LOGI("after %s() glError (0x%x)\n", op, error);
}
class PNGDecoder;

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


GLuint load_png_asset_into_texture(const char* relative_path) {
    assert(relative_path != NULL);

    const FileData png_file = get_asset_data(relative_path);
    PNGDecoder pngDecoder;
    const RawImageData raw_image_data = pngDecoder.get_raw_image_data_from_png_file("/mnt/sdcard/Pictures/image.png");
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

extern "C" JNIEXPORT
void JNICALL Java_com_github_sammyvimes_nativeimageviewlibrary_NativeImageView_native_1start(JNIEnv* UNUSED_ATTR, jclass UNUSED_ATTR) {
    LOGI("START");
}

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


        GLuint content = load_png_file_into_texture("/mnt/sdcard/Pictures/tile.png");
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
void JNICALL Java_com_github_sammyvimes_nativeimageviewlibrary_NativeImageView_native_1surface_1created(JNIEnv* UNUSED_ATTR, jclass UNUSED_ATTR) {
    LOGI("SURFACE CREATED");
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    m_defaultTexture = load_png_file_into_texture("/mnt/sdcard/Pictures/checkers.png");
    refreshBackingStore();
}

extern "C" JNIEXPORT
void JNICALL Java_com_github_sammyvimes_nativeimageviewlibrary_NativeImageView_native_1gl_1render(JNIEnv* UNUSED_ATTR, jclass UNUSED_ATTR) {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // For very fast zooming in and out, do not apply any expensive filter.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // Ensure we would have seamless transition between adjecent tiles.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // This is background, i.e. all the outdated tiles while
    // new primary ones are being prepared (e.g. after zooming).
    if (!m_secondaryBuffer.isEmpty()) {
        QRect backgroundRange = m_secondaryBuffer.visibleRange(m_viewOffset, m_viewZoomFactor, size());
        m_secondaryBuffer.setViewModelMatrix(m_viewOffset, m_viewZoomFactor);

        for (int x = 0; x < m_secondaryBuffer.width(); ++x) {
            for (int y = 0; y < m_secondaryBuffer.height(); ++y) {
                if (backgroundRange.contains(x, y))
                    m_secondaryBuffer.draw(x, y, m_defaultTexture);
                else
                    m_secondaryBuffer.remove(x, y);
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

    m_mainBuffer.setViewModelMatrix(m_viewOffset, m_viewZoomFactor);
    bool needsUpdate = false;
    LOGI("Main Buffer width %d height %d", m_mainBuffer.width(), m_mainBuffer.height());
    for (int x = 0; x < m_mainBuffer.width(); ++x) {
        for (int y = 0; y < m_mainBuffer.height(); ++y) {
            GLuint texture = m_mainBuffer.at(x, y);
//            LOGI("Texture is %d", texture);
            if (updateRange.contains(x, y)) {
                LOGI("Range contains");
                LOGI("Substitute %d", substitute);
                m_mainBuffer.draw(x, y, substitute);
                if (texture == 0)
                    needsUpdate = true;
            }

            // Save GPU memory and throw out unneeded texture
            if (texture != 0 && !updateRange.contains(x, y))
                m_mainBuffer.remove(x, y);
        }
    }

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
    // Ensure that (0,0) is top left and (width - 1, height -1) is bottom right.
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrthof(0, w, h, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}


#endif //NATIVEIMAGEVIEW_NATIVEIMAGEVIEW_H
