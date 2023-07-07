/*
 * kuriborosu
 * Copyright (C) 2021-2023 Filipe Coelho <falktx@falktx.com>
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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[])
{
    // TODO use more advanced opts
    uint32_t opts_buffer_size = 256;
    uint32_t opts_sample_rate = 48000;

    if (argc >= 2 && strcmp(argv[1], "--version") == 0)
    {
        printf("kuriborosu v0.0.0, using Carla v" CARLA_VERSION_STRING "\n"
               "Copyright 2021-2023 Filipe Coelho <falktx@falktx.com>\n"
               "License: ???\n"
               "This is free software: you are free to change and redistribute it.\n"
               "There is NO WARRANTY, to the extent permitted by law.\n");
        return EXIT_SUCCESS;
    }

    if (argc < 4 || strcmp(argv[1], "--help") == 0)
    {
        printf("Usage: kuriborosu [INFILE|NUMSECONDS] OUTFILE PLUGIN1 PLUGIN2... etc\n"
                "Where the first argument can be a filename for input file, or number of seconds to render (useful for self-generators).\n\n"
                "  --help       Display this help and exit\n"
                "  --version    Display version information and exit\n");
        return EXIT_SUCCESS;
    }

    const char* infile = argv[1];
    const char* outwav = argv[2];

    Kuriborosu* const kuri = kuriborosu_host_init(opts_buffer_size, opts_sample_rate);

    if (kuri == NULL)
        return EXIT_FAILURE;

    uint32_t file_frames;

    // Check if input file argument is actually seconds
    // FIXME some isalpha() check??
    const bool isfile = strchr(infile, '.') != NULL || strchr(infile, '/') != NULL;
    if (isfile)
    {
        printf("loading file '%s'...\n", infile);
        if (! kuriborosu_host_load_file(kuri, infile))
            goto error;

        file_frames = (uint32_t)(get_file_length_from_last_plugin(kuri) * opts_sample_rate + 0.5);
        printf("file has %u frames, %g seconds\n", file_frames, (double)file_frames/opts_sample_rate);
    }
    else
    {
        const int seconds = atoi(infile);

        if (seconds <= 0 || seconds > 60*60)
        {
            fprintf(stderr, "Invalid number of seconds %i\n", seconds);
            goto error;
        }

        file_frames = (uint32_t)seconds * opts_sample_rate;
    }

    if (file_frames > 60*60*opts_sample_rate)
    {
        fprintf(stderr, "Output file unexpectedly big, bailing out\n");
        goto error;
    }

    for (int i = 3; i < argc; ++i)
    {
        const char* const plugin_arg = argv[i];
        printf("%d %s\n", i, plugin_arg);

        // check if file
        if (plugin_arg[0] == '.' || plugin_arg[0] == '/')
        {
            kuriborosu_host_load_file(kuri, plugin_arg);
        }
        // check if argument
        else if (plugin_arg[0] == '-')
        {
            switch (plugin_arg[1])
            {
            case 'p':
                // TODO complete me
                if (++i < argc)
                {
                    kuriborosu_host_set_plugin_custom_data(kuri, CUSTOM_DATA_TYPE_PATH, "file", argv[i]);
                }
                break;
            default:
                // TODO give error
                break;
            }
        }
        else
        {
            kuriborosu_host_load_plugin(kuri, plugin_arg);
        }
    }

    const file_render_options_t options = {
        .filename = outwav,
        .frames = file_frames,
        .tail_mode = isfile ? tail_mode_continue_until_silence : tail_mode_none,
    };
    kuriborosu_host_render_to_file(kuri, &options);

    kuriborosu_host_destroy(kuri);
    return EXIT_SUCCESS;

error:
    kuriborosu_host_destroy(kuri);
    return EXIT_FAILURE;
}
