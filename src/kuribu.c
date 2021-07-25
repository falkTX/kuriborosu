/*
 * kuriborosu
 * Copyright (C) 2021 Filipe Coelho <falktx@falktx.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_help()
{
    printf("Usage: kuribu [INFILE|NUMSECONDS] OUTFILE PLUGIN1 PLUGIN2... etc\n"
           "Where the first argument can be a filename for input file, or number of seconds to render (useful for self-generators).\n\n"
           "  --help       Display this help and exit\n"
           "  --version    Display version information and exit\n");
}

static void print_version()
{
    printf("kuribu v0.0.0, using Carla v" CARLA_VERSION_STRING "\n"
           "Copyright 2021 Filipe Coelho <falktx@falktx.com>\n"
           "License: ???\n"
           "This is free software: you are free to change and redistribute it.\n"
           "There is NO WARRANTY, to the extent permitted by law.\n");
}

int main(int argc, char* argv[])
{
    if (argc < 4)
    {
        print_help();
        return EXIT_FAILURE;
    }

    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "--help") == 0)
        {
            print_version();
            return EXIT_SUCCESS;
        }
        if (strcmp(argv[i], "--version") == 0)
        {
            print_version();
            return EXIT_SUCCESS;
        }
    }

    const char* infile = argv[1];
    const char* outwav = argv[2];

    const uint32_t buffer_size = 1024;
    const uint32_t sample_rate = 48000;

    Kuriborosu* const kuri = kuriborosu_host_init(buffer_size, sample_rate);

    if (kuri == NULL)
        return EXIT_FAILURE;

    uint32_t file_frames;

    // Check if input file argument is actually seconds
    // FIXME some isalpha() check??
    const bool isfile = strchr(infile, '.') != NULL || strchr(infile, '/') != NULL;
    if (isfile)
    {
        if (! kuriborosu_host_load_file(kuri, infile))
            goto error;

        file_frames = (uint32_t)(get_file_length_from_last_plugin(kuri) * sample_rate + 0.5);
    }
    else
    {
        const int seconds = atoi(infile);

        if (seconds <= 0 || seconds > 60*60)
        {
            fprintf(stderr, "Invalid number of seconds %i\n", seconds);
            goto error;
        }

        file_frames = (uint32_t)seconds * sample_rate;
    }

    if (file_frames > 60*60*sample_rate)
    {
        fprintf(stderr, "Output file unexpectedly big, bailing out\n");
        goto error;
    }

    for (int i = 3; i < argc; ++i)
    {
        const char* const plugin_arg = argv[i];

        // check if file
        if (plugin_arg[0] == '.' || plugin_arg[0] == '/')
            kuriborosu_host_load_file(kuri, plugin_arg);
        else
            kuriborosu_host_load_plugin(kuri, plugin_arg);
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
