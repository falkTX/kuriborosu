// TODO license header here

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
bool kuriborosu_host_render_to_file(Kuriborosu* kuri, const file_render_options_t* options);

double get_file_length_from_last_plugin(Kuriborosu* kuri);
