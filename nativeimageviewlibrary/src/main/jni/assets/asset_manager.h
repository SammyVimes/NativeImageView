//
// Created by Semyon on 10.01.2016.
//

#ifndef NATIVEIMAGEVIEW_ASSET_MANAGER_H
#define NATIVEIMAGEVIEW_ASSET_MANAGER_H

#define UNUSED(x) (void)(x)

#include "../file/platform_file_utils.h"

FileData get_asset_data(const char* relative_path);
void release_asset_data(const FileData* file_data);

#endif //NATIVEIMAGEVIEW_ASSET_MANAGER_H
