/*
 * kuriborosu
 * Copyright (C) 2021 Filipe Coelho <falktx@falktx.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Affero General Public License for more details.
 *
 * For a full copy of the GNU Affero General Public License see LICENSE file.
 */

#pragma once

#include "CarlaNativePlugin.h"

typedef struct _Kuriborosu Kuriborosu;

typedef enum tail_mode_t {
    // no tail
    tail_mode_none,
    // keep going a bit until silence, maximum 5 seconds
    tail_mode_continue_until_silence,
    // loop once, TODO
    tail_mode_looping
} tail_mode_t;

typedef struct FILE_RENDER_OPTIONS_T {
    const char* filename;
    uint32_t frames;
    tail_mode_t tail_mode;
} file_render_options_t;

Kuriborosu* kuriborosu_host_init(uint32_t buffer_size, uint32_t sample_rate);
void kuriborosu_host_destroy(Kuriborosu* kuri);

bool kuriborosu_host_load_file(Kuriborosu* kuri, const char* filename);
bool kuriborosu_host_load_plugin(Kuriborosu* kuri, const char* filenameOrUID);
bool kuriborosu_host_set_plugin_custom_data(Kuriborosu* kuri, const char* type, const char* key, const char* value);
bool kuriborosu_host_render_to_file(Kuriborosu* kuri, const file_render_options_t* options);

double get_file_length_from_last_plugin(Kuriborosu* kuri);
