//
// Created by Semyon on 10.01.2016.
//
#pragma once
#ifndef NATIVEIMAGEVIEW_ASSET_HELPER_H
#define NATIVEIMAGEVIEW_ASSET_HELPER_H


#include <stdlib.h>
#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <assert.h>
#include "platform_file_utils.h"

#define UNUSED(x) (void)(x)

FileData get_asset_data(const char* relative_path);

void release_asset_data(const FileData* file_data);


#endif //NATIVEIMAGEVIEW_ASSET_HELPER_H
