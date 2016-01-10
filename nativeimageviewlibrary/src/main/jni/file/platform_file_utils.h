//
// Created by Semyon on 10.01.2016.
//

#pragma once

typedef struct {
    const long data_length;
    const void* data;
    const void* file_handle;
} FileData;


FileData get_file_data(const char* path);
void release_file_data(const FileData* file_data);