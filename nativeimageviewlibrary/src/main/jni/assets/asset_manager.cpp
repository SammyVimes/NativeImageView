//
// Created by Semyon on 10.01.2016.
//

#include "asset_manager.h"

#include "../gl/platform_log.h"
#include <android/asset_manager_jni.h>
#include <assert.h>

static AAssetManager* asset_manager;

JNIEXPORT void JNICALL
Java_com_github_sammyvimes_nativeimageviewlibrary_NativeAssetManager_init_1asset_1manager(
        JNIEnv *env, jclass type, jobject assetManager) {
    UNUSED(type);
    asset_manager = AAssetManager_fromJava(env, assetManager);
}

FileData get_asset_data(const char* relative_path) {
    assert(relative_path != NULL);
    AAsset* asset = AAssetManager_open(asset_manager, relative_path, AASSET_MODE_STREAMING);
    assert(asset != NULL);

    return (FileData) { AAsset_getLength(asset), AAsset_getBuffer(asset), asset };
}

void release_asset_data(const FileData* file_data) {
    assert(file_data != NULL);
    assert(file_data->file_handle != NULL);
    AAsset_close((AAsset*)file_data->file_handle);
}
