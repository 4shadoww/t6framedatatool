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

#ifndef ARG_PARSER_HPP
#define ARG_PARSER_HPP

struct Arg;

struct Configuration {
    int log_level;
    bool frame_data_logging;
};

class ArgParser {
private:
    static Configuration *s_configuration;
    static const char *s_program_name;
    static Arg s_arguments[];

    static int arg_print_help(const char * /*value*/);
    static int arg_print_version(const char * /*value*/);
    static int arg_verbose(const char * /*value*/);
    static int arg_print_frames(const char * /*value*/);

public:
    static Configuration create_default_config();
    static int parse_arguments(const int argc, const char **argv, Configuration *config);
};

#endif
