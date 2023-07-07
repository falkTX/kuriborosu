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

#include "host.h"

#include <float.h>
#include <math.h>

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sndfile.h>

typedef struct _Kuriborosu {
    uint32_t buffer_size;
    uint32_t sample_rate;
    const NativePluginDescriptor* plugin_descriptor;
    NativePluginHandle plugin_handle;
    NativeHostDescriptor host_descriptor;
    CarlaHostHandle carla_handle;
    NativeTimeInfo time;
    bool plugin_needs_idle;
} Kuriborosu;

#define kuriborosu ((Kuriborosu*)handle)

static uint32_t get_buffer_size(const NativeHostHandle handle)
{
    return kuriborosu->buffer_size;
}

static double get_sample_rate(const NativeHostHandle handle)
{
    return kuriborosu->sample_rate;
}

static bool is_offline(const NativeHostHandle handle)
{
    return true;

    // unused
    (void)handle;
}

static const NativeTimeInfo* get_time_info(const NativeHostHandle handle)
{
    return &kuriborosu->time;
}

static bool write_midi_event(const NativeHostHandle handle, const NativeMidiEvent* const event)
{
    return false;

    // unused
    (void)handle;
    (void)event;
}

static void ui_parameter_changed(const NativeHostHandle handle, const uint32_t index, const float value)
{
    return;

    // unused
    (void)handle;
    (void)index;
    (void)value;
}

static void ui_midi_program_changed(const NativeHostHandle handle, const uint8_t channel, const uint32_t bank, const uint32_t program)
{
    return;

    // unused
    (void)handle;
    (void)channel;
    (void)bank;
    (void)program;
}

static void ui_custom_data_changed(const NativeHostHandle handle, const char* const key, const char* const value)
{
    return;

    // unused
    (void)handle;
    (void)key;
    (void)value;
}

static void ui_closed(const NativeHostHandle handle)
{
    return;

    // unused
    (void)handle;
}

static const char* ui_open_file(const NativeHostHandle handle, const bool isDir, const char* const title, const char* const filter)
{
    // not supported
    return NULL;

    // unused
    (void)handle;
    (void)isDir;
    (void)title;
    (void)filter;
}

static const char* ui_save_file(const NativeHostHandle handle, const bool isDir, const char* const title, const char* const filter)
{
    // not supported
    return NULL;

    // unused
    (void)handle;
    (void)isDir;
    (void)title;
    (void)filter;
}

static intptr_t dispatcher(const NativeHostHandle handle,
                           const NativeHostDispatcherOpcode opcode, const int32_t index, const intptr_t value, void* const ptr, const float opt)
{
    switch (opcode)
    {
    case NATIVE_HOST_OPCODE_REQUEST_IDLE:
        kuriborosu->plugin_needs_idle = true;
        return 1;
    default:
        break;
    }

    return 0;

    // unused
    (void)index;
    (void)value;
    (void)ptr;
    (void)opt;
}

#undef kuriborosu

static void carla_stderr2(const char* const fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "\x1b[31m");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\x1b[0m\n");
    fflush(stderr);
    va_end(args);
}

static void carla_safe_assert(const char* const assertion, const char* const file, const int line)
{
    carla_stderr2("Kuriborosu assertion failure: \"%s\" in file %s, line %i", assertion, file, line);
}

Kuriborosu* kuriborosu_host_init(const uint32_t buffer_size, const uint32_t sample_rate)
{
    Kuriborosu* const kuri = (Kuriborosu*)malloc(sizeof(Kuriborosu));

    if (kuri == NULL)
        return NULL;

    memset(kuri, 0, sizeof(Kuriborosu));
    kuri->buffer_size = buffer_size;
    kuri->sample_rate = sample_rate;

    kuri->host_descriptor.handle = kuri;
    kuri->host_descriptor.resourceDir = carla_get_library_folder();
    kuri->host_descriptor.get_buffer_size = get_buffer_size;
    kuri->host_descriptor.get_sample_rate = get_sample_rate;
    kuri->host_descriptor.is_offline = is_offline;
    kuri->host_descriptor.get_time_info = get_time_info;
    kuri->host_descriptor.write_midi_event = write_midi_event;
    kuri->host_descriptor.ui_parameter_changed = ui_parameter_changed;
    kuri->host_descriptor.ui_midi_program_changed = ui_midi_program_changed;
    kuri->host_descriptor.ui_custom_data_changed = ui_custom_data_changed;
    kuri->host_descriptor.ui_closed = ui_closed;
    kuri->host_descriptor.ui_open_file = ui_open_file;
    kuri->host_descriptor.ui_save_file = ui_save_file;
    kuri->host_descriptor.dispatcher = dispatcher;

    kuri->plugin_descriptor = carla_get_native_rack_plugin();

    if (kuri->plugin_descriptor == NULL)
    {
        fprintf(stderr, "Failed to load Carla-Rack plugin\n");
        goto error;
    }

    kuri->plugin_handle = kuri->plugin_descriptor->instantiate(&kuri->host_descriptor);

    if (kuri->plugin_handle == NULL)
    {
        fprintf(stderr, "Failed to instantiate Carla-Rack plugin\n");
        goto error;
    }

    kuri->carla_handle = carla_create_native_plugin_host_handle(kuri->plugin_descriptor,
                                                                kuri->plugin_handle);

    if (kuri->carla_handle == NULL)
    {
        fprintf(stderr, "Failed to create Carla-Rack host handle\n");
        goto cleanup;
    }

    kuri->plugin_descriptor->activate(kuri->plugin_handle);

    return kuri;

cleanup:
    kuri->plugin_descriptor->cleanup(kuri->plugin_handle);

error:
    free(kuri);
    return NULL;
}

void kuriborosu_host_destroy(Kuriborosu* const kuri)
{
    CARLA_SAFE_ASSERT_RETURN(kuri != NULL,);

    kuri->plugin_descriptor->deactivate(kuri->plugin_handle);
    kuri->plugin_descriptor->cleanup(kuri->plugin_handle);
    carla_host_handle_free(kuri->carla_handle);
    free(kuri);
}

bool kuriborosu_host_load_file(Kuriborosu* const kuri, const char* const filename)
{
    CARLA_SAFE_ASSERT_RETURN(kuri != NULL, false);
    CARLA_SAFE_ASSERT_RETURN(filename != NULL, false);

    const uint32_t plugin_id = carla_get_current_plugin_count(kuri->carla_handle);

    if (carla_load_file(kuri->carla_handle, filename))
    {
        // Disable audiofile looping
        if (strcmp(carla_get_real_plugin_name(kuri->carla_handle, plugin_id), "Audio File") == 0)
        {
            const uint32_t parameter_count = carla_get_parameter_count(kuri->carla_handle, plugin_id);

            for (uint32_t i=0; i<parameter_count; ++i)
            {
                const CarlaParameterInfo* const info = carla_get_parameter_info(kuri->carla_handle, plugin_id, i);

                if (strcmp(info->name, "Loop Mode") == 0)
                {
                    carla_set_parameter_value(kuri->carla_handle, plugin_id, i, 0.0f);
                    break;
                }
            }
        }

        return true;
    }

    fprintf(stderr, "Failed to load file %s, error was: %s\n",
            filename, carla_get_last_error(kuri->carla_handle));
    return false;
}

bool kuriborosu_host_set_plugin_custom_data(Kuriborosu* kuri, const char* type, const char* key, const char* value)
{
    CARLA_SAFE_ASSERT_RETURN(kuri != NULL, false);
    CARLA_SAFE_ASSERT_RETURN(type != NULL, false);
    CARLA_SAFE_ASSERT_RETURN(key != NULL, false);
    CARLA_SAFE_ASSERT_RETURN(value != NULL, false);

    const uint32_t plugin_count = carla_get_current_plugin_count(kuri->carla_handle);
    CARLA_SAFE_ASSERT_RETURN(plugin_count != 0, false);

    const uint32_t plugin_id = plugin_count - 1;

    carla_set_custom_data(kuri->carla_handle, plugin_id, type, key, value);
    printf("set custom data '%s'\n", value);
    return true;
}

bool kuriborosu_host_load_plugin(Kuriborosu* kuri, const char* filenameOrUID)
{
    CARLA_SAFE_ASSERT_RETURN(kuri != NULL, false);
    CARLA_SAFE_ASSERT_RETURN(filenameOrUID != NULL, false);

    if (carla_add_plugin(kuri->carla_handle, BINARY_NATIVE, PLUGIN_LV2, "", "", filenameOrUID, 0, NULL, PLUGIN_OPTIONS_NULL))
        return true;

    fprintf(stderr, "Failed to load plugin %s, error was: %s\n",
            filenameOrUID, carla_get_last_error(kuri->carla_handle));
    return false;
}

bool kuriborosu_host_render_to_file(Kuriborosu* const kuri, const file_render_options_t* const options)
{
    CARLA_SAFE_ASSERT_RETURN(kuri != NULL, false);
    CARLA_SAFE_ASSERT_RETURN(options != NULL, false);

    const uint32_t buffer_size = kuri->buffer_size;
    const uint32_t sample_rate = kuri->sample_rate;

    float* const bufN = malloc(sizeof(float)*buffer_size*2);
    float* const bufL = malloc(sizeof(float)*buffer_size);
    float* const bufR = malloc(sizeof(float)*buffer_size);

    if (bufN == NULL || bufL == NULL || bufR == NULL)
    {
        fprintf(stderr, "Out of memory\n");
        return false;
    }

    SF_INFO sf_fmt = {
        .frames = 0,
        .samplerate = sample_rate,
        .channels = 2,
        .format = SF_FORMAT_WAV|SF_FORMAT_PCM_16,
        .sections = 0,
        .seekable = 0,
    };
    SNDFILE* const file = sf_open(options->filename, SFM_WRITE, &sf_fmt);

    // TODO check file NULL or error

    if (file == NULL)
    {
        goto free;
    }

    // Turn on clipping and normalization of floats (-1.0 - 1.0)
    sf_command(file, SFC_SET_CLIPPING, NULL, SF_TRUE);
    sf_command(file, SFC_SET_NORM_FLOAT, NULL, SF_TRUE);

    float* inbuf[2] = { bufN, bufN + buffer_size };
    float* outbuf[2] = { bufL, bufR };

    kuri->time.playing = true;
    kuri->time.bbt.valid = true;
    kuri->time.bbt.beatsPerBar    = 4;
    kuri->time.bbt.beatType       = 4;
    kuri->time.bbt.ticksPerBeat   = 1920;
    kuri->time.bbt.beatsPerMinute = 120;

    // start
    kuri->time.bbt.bar  = 1;
    kuri->time.bbt.beat = 1;

    for (uint32_t i = 0; i < options->frames; i += buffer_size)
    {
        kuri->time.frame = i;
        memset(bufN, 0, sizeof(float)*buffer_size*2);
        kuri->plugin_descriptor->process(kuri->plugin_handle, inbuf, outbuf, buffer_size, NULL, 0);

        // interleave
        for (uint32_t j = 0, k = 0; k < buffer_size; j += 2, ++k)
        {
            bufN[j+0] = bufL[k];
            bufN[j+1] = bufR[k];
        }

        sf_writef_float(file, bufN, buffer_size);

        if (kuri->plugin_needs_idle)
        {
            kuri->plugin_needs_idle = false;
            kuri->plugin_descriptor->dispatcher(kuri->plugin_handle, NATIVE_PLUGIN_OPCODE_IDLE, 0, 0, NULL, 0.0f);
        }

        // move BBT forwards
        double newtick = kuri->time.bbt.tick
             + (buffer_size * kuri->time.bbt.ticksPerBeat * kuri->time.bbt.beatsPerMinute / (sample_rate * 60));

        while (newtick >= kuri->time.bbt.ticksPerBeat)
        {
            newtick -= kuri->time.bbt.ticksPerBeat;

            if (++kuri->time.bbt.beat > kuri->time.bbt.beatsPerBar)
            {
                ++kuri->time.bbt.bar;
                kuri->time.bbt.beat = 1;
                kuri->time.bbt.barStartTick += kuri->time.bbt.beatsPerBar * kuri->time.bbt.ticksPerBeat;
            }
        }

        kuri->time.bbt.tick = newtick;
    }

    if (options->tail_mode == tail_mode_continue_until_silence)
    {
        // keep going a bit until silence, maximum 5 seconds
        const uint32_t until_silence = 5 * sample_rate;
        kuri->time.playing = false;

        for (uint32_t i = 0; i < until_silence; i += buffer_size)
        {
            memset(bufN, 0, sizeof(float)*buffer_size*2);
            kuri->plugin_descriptor->process(kuri->plugin_handle, inbuf, outbuf, buffer_size, NULL, 0);

            // interleave
            for (uint32_t j = 0, k = 0; k < buffer_size; j += 2, ++k)
            {
                bufN[j+0] = bufL[k];
                bufN[j+1] = bufR[k];
            }

            sf_writef_float(file, bufN, buffer_size);

            if (kuri->plugin_needs_idle)
            {
                kuri->plugin_needs_idle = false;
                kuri->plugin_descriptor->dispatcher(kuri->plugin_handle, NATIVE_PLUGIN_OPCODE_IDLE, 0, 0, NULL, 0.0f);
            }

            if (fabsf(bufN[buffer_size-1]) < __FLT_EPSILON__)
                break;
        }
    }

free:
    free(bufN);
    free(bufL);
    free(bufR);
    sf_close(file);

    return true;
}

double get_file_length_from_last_plugin(Kuriborosu* const kuri)
{
    static const double fallback = 60.0;
    CARLA_SAFE_ASSERT_RETURN(kuri != NULL, fallback);

    const uint32_t next_plugin_id = carla_get_current_plugin_count(kuri->carla_handle);
    CARLA_SAFE_ASSERT_RETURN(next_plugin_id != 0, fallback);

    const uint32_t plugin_id = next_plugin_id - 1;
    const char* const plugin_name = carla_get_real_plugin_name(kuri->carla_handle, plugin_id);
    CARLA_SAFE_ASSERT_RETURN(plugin_name != NULL, fallback);

    if (strcmp(plugin_name, "Audio File") == 0 || strcmp(plugin_name, "MIDI File") == 0)
    {
        const uint32_t parameter_count = carla_get_parameter_count(kuri->carla_handle, plugin_id);

        for (uint32_t i=0; i<parameter_count; ++i)
        {
            const CarlaParameterInfo* const info = carla_get_parameter_info(kuri->carla_handle, plugin_id, i);

            if (strcmp(info->name, "Length") == 0)
                return carla_get_current_parameter_value(kuri->carla_handle, plugin_id, i);
        }
    }

    return fallback;
}
