// TODO license header here

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sndfile.h>

#include "CarlaNativePlugin.h"

typedef struct {
    uint32_t buffer_size;
    double sample_rate;
    NativeTimeInfo time;
    bool plugin_needs_idle;
} Koriborosu;

#define koriborosu ((Koriborosu*)handle)

static uint32_t get_buffer_size(NativeHostHandle handle)
{
    return koriborosu->buffer_size;
}

static double get_sample_rate(NativeHostHandle handle)
{
    return koriborosu->sample_rate;
}

static bool is_offline(NativeHostHandle handle)
{
    return true;
}

static const NativeTimeInfo* get_time_info(NativeHostHandle handle)
{
    return &koriborosu->time;
}

static bool write_midi_event(NativeHostHandle handle, const NativeMidiEvent* event)
{
    return false;
}

static void ui_parameter_changed(NativeHostHandle handle, uint32_t index, float value)
{
}

static void ui_midi_program_changed(NativeHostHandle handle, uint8_t channel, uint32_t bank, uint32_t program)
{
}

static void ui_custom_data_changed(NativeHostHandle handle, const char* key, const char* value)
{
}

static void ui_closed(NativeHostHandle handle)
{
}

static const char* ui_open_file(NativeHostHandle handle, bool isDir, const char* title, const char* filter)
{
    // not supported
    return NULL;
}

static const char* ui_save_file(NativeHostHandle handle, bool isDir, const char* title, const char* filter)
{
    // not supported
    return NULL;
}

static intptr_t dispatcher(NativeHostHandle handle,
                           NativeHostDispatcherOpcode opcode, int32_t index, intptr_t value, void* ptr, float opt)
{
    switch (opcode)
    {
    case NATIVE_HOST_OPCODE_REQUEST_IDLE:
        koriborosu->plugin_needs_idle = true;
        return 1;
    default:
        break;
    }

    return 0;
}

#undef koriborosu

static double get_file_length_from_plugin(const CarlaHostHandle handle)
{
    if (strcmp(carla_get_real_plugin_name(handle, 0), "Audio File") == 0)
        return carla_get_current_parameter_value(handle, 0, 5 /* kParameterInfoLength */);

    // TODO read from midi plugin

    // default value
    return 60.0;
}

int main(int argc, char* argv[])
{
    // TODO use more advanced opts
    uint32_t opts_buffer_size = 1024;
    uint32_t opts_sample_rate = 48000;

    if (argc >= 2 && strcmp(argv[1], "--version") == 0)
    {
        printf("koriborosu v0.0.0, using Carla v" CARLA_VERSION_STRING "\n"
               "Copyright 2021 Filipe Coelho <falktx@falktx.com>\n"
               "License: ???\n"
               "This is free software: you are free to change and redistribute it.\n"
               "There is NO WARRANTY, to the extent permitted by law.\n");
        return EXIT_SUCCESS;
    }

    if (argc < 4 || strcmp(argv[1], "--help") == 0)
    {
        printf("Usage: koriborosu [INFILE|NUMSECONDS] OUTFILE PLUGIN1 PLUGIN2... etc\n"
                "Where the first argument can be a filename for input file, or number of seconds to render (useful for self-generators).\n\n"
                "  --help       Display this help and exit\n"
                "  --version    Display version information and exit\n");
        return EXIT_SUCCESS;
    }

    const char* infile = argv[1];
    const char* outwav = argv[2];

    Koriborosu kori;
    memset(&kori, 0, sizeof(kori));
    kori.buffer_size = opts_buffer_size;
    kori.sample_rate = opts_sample_rate;

    NativeHostDescriptor hdesc;
    memset(&hdesc, 0, sizeof(hdesc));
    hdesc.handle = &kori;
    hdesc.resourceDir = carla_get_library_folder();
    hdesc.get_buffer_size = get_buffer_size;
    hdesc.get_sample_rate = get_sample_rate;
    hdesc.is_offline = is_offline;
    hdesc.get_time_info = get_time_info;
    hdesc.write_midi_event = write_midi_event;
    hdesc.ui_parameter_changed = ui_parameter_changed;
    hdesc.ui_midi_program_changed = ui_midi_program_changed;
    hdesc.ui_custom_data_changed = ui_custom_data_changed;
    hdesc.ui_closed = ui_closed;
    hdesc.ui_open_file = ui_open_file;
    hdesc.ui_save_file = ui_save_file;
    hdesc.dispatcher = dispatcher;

    const NativePluginDescriptor* const pdesc = carla_get_native_rack_plugin();

    if (pdesc == NULL)
    {
        fprintf(stderr, "Failed to load Carla-Rack plugin\n");
        return EXIT_FAILURE;
    }

    const NativePluginHandle phandle = pdesc->instantiate(&hdesc);

    if (phandle == NULL)
    {
        fprintf(stderr, "Failed to instantiate Carla-Rack plugin\n");
        return EXIT_FAILURE;
    }

    pdesc->activate(&hdesc);

    if (phandle == NULL)
    {
        fprintf(stderr, "Failed to activate Carla-Rack plugin\n");
        pdesc->cleanup(phandle);
        return EXIT_FAILURE;
    }

    int ret = EXIT_FAILURE;

    const CarlaHostHandle hhandle = carla_create_native_plugin_host_handle(pdesc, phandle);

    if (hhandle == NULL)
    {
        fprintf(stderr, "Failed to create Carla-Rack host handle\n");
        goto deactivate;
    }

    uint32_t file_frames;

    // Check if input file argument is actually seconds
    // FIXME some isalpha() check??
    if (strchr(infile, '.') == NULL && strchr(infile, '/') == NULL)
    {
        const int seconds = atoi(infile);

        if (seconds <= 0 || seconds > 60*60)
        {
            fprintf(stderr, "Invalid number of seconds %i\n", seconds);
            goto deactivate;
        }

        file_frames = (uint32_t)seconds * opts_sample_rate;
    }
    else
    {
        if (! carla_load_file(hhandle, infile))
        {
            fprintf(stderr, "Failed to load input file, error was: %s\n", carla_get_last_error(hhandle));
            goto deactivate;
        }

#if 0
        // TESTING reduce overall volume of audio file to prevent clipping
        if (strcmp(carla_get_real_plugin_name(hhandle, 0), "Audio File") == 0)
            carla_set_volume(hhandle, 0, 0.5f);
#endif

        file_frames = (uint32_t)(get_file_length_from_plugin(hhandle) * opts_sample_rate + 0.5);
    }

    for (int i = 3; i < argc; ++i)
    {
        if (! carla_add_plugin(hhandle, BINARY_NATIVE, PLUGIN_LV2, "", "", argv[i], 0, NULL, 0x0))
            fprintf(stderr, "Failed to load plugin %s, error was: %s\n", argv[i], carla_get_last_error(hhandle));
    }

    SF_INFO sf_fmt = {
        .frames = 0,
        .samplerate = opts_sample_rate,
        .channels = 2,
        .format = SF_FORMAT_WAV|SF_FORMAT_PCM_16,
        .sections = 0,
        .seekable = 0,
    };
    SNDFILE* const file = sf_open(outwav, SFM_WRITE, &sf_fmt);

    // TODO check file NULL or error

    // Turn on clipping and normalization of floats (-1.0 - 1.0)
    sf_command(file, SFC_SET_CLIPPING, NULL, SF_TRUE);
    sf_command(file, SFC_SET_NORM_FLOAT, NULL, SF_TRUE);

    float* bufN = malloc(sizeof(float)*opts_buffer_size*2);
    float* bufL = malloc(sizeof(float)*opts_buffer_size);
    float* bufR = malloc(sizeof(float)*opts_buffer_size);

    if (bufN == NULL || bufL == NULL || bufR == NULL)
        goto free;

    float* inbuf[2] = { bufN, bufN + opts_buffer_size };
    float* outbuf[2] = { bufL, bufR };

    kori.time.playing = true;

    for (uint32_t i = 0; i < file_frames; i += opts_buffer_size)
    {
        kori.time.frame = i;
        memset(bufN, 0, sizeof(float)*opts_buffer_size*2);
        pdesc->process(phandle, inbuf, outbuf, opts_buffer_size, NULL, 0);

        // interleave
        for (uint32_t j = 0, k = 0; k < opts_buffer_size; j += 2, ++k)
        {
            bufN[j+0] = bufL[k];
            bufN[j+1] = bufR[k];
        }

        sf_writef_float(file, bufN, opts_buffer_size);

        if (kori.plugin_needs_idle)
        {
            kori.plugin_needs_idle = false;
            pdesc->dispatcher(phandle, NATIVE_PLUGIN_OPCODE_IDLE, 0, 0, NULL, 0.0f);
        }
    }

    ret = EXIT_SUCCESS;

free:
    free(bufN);
    free(bufL);
    free(bufR);
    sf_close(file);

deactivate:
    pdesc->deactivate(phandle);
    pdesc->cleanup(phandle);
    carla_host_handle_free(hhandle);
    return ret;
}
