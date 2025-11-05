/*
  Copyright (C) 2025 Noa-Emil Nissinen

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.    See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.    If not, see <https://www.gnu.org/licenses/>.
*/

#include <cstdint>
#include <cstring>
#include <iostream>

#include "arg_parser.hpp"

#include "logging.h"
#include "version.hpp"

struct Arg {
    char long_form[25];
    char short_form[5];
    int type;
    int (*handler)(const char *);
};

enum ArgType : uint8_t {
    FLAG,
    VALUE
};

Configuration *ArgParser::s_configuration = {};
const char *ArgParser::s_program_name = {};

Arg ArgParser::s_arguments[] = {
    {.long_form = "--help", .short_form = "-h", .type = ArgType::FLAG, .handler = &ArgParser::arg_print_help},
    {.long_form = "--version", .short_form = "\0", .type = ArgType::FLAG, .handler = &arg_print_version},
    {.long_form = "--verbose", .short_form = "-v", .type = ArgType::FLAG, .handler = &arg_verbose},
    {.long_form = "--print-frames", .short_form = "-pf", .type = ArgType::FLAG, .handler = &arg_print_frames},
};

int ArgParser::arg_print_help(const char * /*value*/) {
    char options[] = "\nOptions:\n"
                     "  -h,   --help\t\t\tprint help\n"
                     "        --version\t\tprint version\n"
                     "  -v,   --verbose\t\tverbose output\n"
                     "  -pf,  --print-frames\t\tprint frame data\n"
                     "\nTekken 6 frame data tool overlay";

    std::cout << "usage: " << s_program_name << " [OPTIONS...]\n" << options << std::endl;

    return -1;
}

int ArgParser::arg_print_version(const char * /*value*/) {
    std::cout << PROGRAM_NAME << " " << VERSION << std::endl;
    return -1;
}

int ArgParser::arg_verbose(const char * /*value*/) {
    s_configuration->log_level = LOG_DEBUG;
    return 0;
}

int ArgParser::arg_print_frames(const char * /*value*/) {
    s_configuration->frame_data_logging = true;
    return 0;
}

Configuration ArgParser::create_default_config() {
    return {.log_level = LOG_INFO, .frame_data_logging = false};
}

int ArgParser::parse_arguments(const int argc, const char **argv, Configuration *config) {
    s_configuration = config;
    s_program_name = argv[0];

    for (int i = 1; i < argc; i++) {
        int success = 0;
        bool arg_parsed = false;
        for (const auto &arg : s_arguments) {
            if (strcmp(argv[i], arg.long_form) != 0 && strcmp(argv[i], arg.short_form) != 0) {
                continue;
            }

            if (arg.type == FLAG) {
                success = arg.handler(nullptr);
                arg_parsed = true;
                break;
            } else if (arg.type == VALUE) {
                if (i + 1 == argc) {
                    std::cerr << "too frew arguments given" << std::endl;
                    return 1;
                }
                i++;
                success = arg.handler(argv[i]);
                arg_parsed = true;
                break;
            }
        }
        if (!arg_parsed) {
            std::cerr << "unknown argument: " << '"' << argv[i] << "'" << std::endl;
            return -1;
        }

        if (success == 1) {
            std::cerr << "failed to parse argument: " << '"' << argv[i] << '"' << std::endl;
            return 1;
        } else if (success == -1) {
            return -1;
        }
    }
    return 0;
}
