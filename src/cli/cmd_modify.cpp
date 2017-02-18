#include <stdint.h>
#include <stddef.h>
#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>

#include "args.hxx"

#include "../headerfile.hpp"
#include "../path.hpp"
#include "../errors.hpp"
#include "../common.hpp"
#include "shared.hpp"

typedef uint32_t flags_t;

void modify_texture(const std::string& texture_name, PegHeader& header,
    const std::string& new_name, uint16_t flags);

static const char* HELP_MODIFY =
R"(
Modifies metadata of a texture.

Usage: % [options] <header> <texture>

Options:

  -h, --help                        Display this help menu
  -o [output], --output=[output]    Directory to write the new container to

  -n [name], --name=[name]          New name of the texture
  -f [flags], --flags=[flags]       Flags to set

  header                            Header file ending with cvbm_pc or cpeg_pc
  texture                           Texture to modify

)";

int cmd_modify(std::string progname,
    std::vector<std::string>::const_iterator beginargs,
    std::vector<std::string>::const_iterator endargs)
{
    progname += " m";
    args::ArgumentParser parser("");
    args::HelpFlag help(parser, "help", "", {'h', "help"});
    args::Positional<std::string> header_arg(parser, "header", "");
    args::Positional<std::string> texture_arg(parser, "texture", "");
    args::ValueFlag<std::string> output_arg(parser, "output", "", {'o', "output"});
    args::ValueFlag<std::string> name_arg(parser, "name", "", {'n', "name"});
    args::ValueFlag<std::string> flags_arg(parser, "flags", "", {'f', "flags"});

    try {
        parser.ParseArgs(beginargs, endargs);
    } catch (args::Help) {
        std::cerr << help_format(HELP_MODIFY, progname);
        return 0;
    } catch (const args::ParseError& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << help_format(HELP_MODIFY, progname);
        return 1;
    } catch (const args::ValidationError& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    if (!header_arg) {
        std::cerr << help_format(HELP_MODIFY, progname);
        return 1;
    }
    if (!texture_arg) {
        errormsg() << "Texture argument is missing" << std::endl;
        std::cerr << help_format(HELP_MODIFY, progname);
        return 1;
    }

    uint16_t flags;
    if (flags_arg) {
        std::string flags_str = args::get(flags_arg);
        try {
            flags = std::stoul(flags_str, 0, 0);
        } catch (std::invalid_argument) {
            errormsg() << "Failed to parse flags argument" << std::endl;
            return 1;
        } catch (std::out_of_range) {
            errormsg() << "Flags argument out of range" << std::endl;
            return 1;
        }
    } else {
        flags = 0xFFFF;
    }

    std::string header_in_filename = args::get(header_arg);
    std::string data_in_filename = get_data_filename(header_in_filename);
    if (data_in_filename.empty()) {
        errormsg() << "Invalid file extension" << std::endl;
        return 1;
    }

    std::string output_dir = args::get(output_arg);
    std::string header_out_filename;
    std::string data_out_filename;

    if (!output_dir.empty()) {
        header_out_filename = path::join(
            output_dir, path::basename(header_in_filename));
        data_out_filename = path::join(
            output_dir, path::basename(data_in_filename));
    } else {
        header_out_filename = header_in_filename;
        data_out_filename = data_in_filename;
    }

    std::string texture_name = args::get(texture_arg);
    std::string new_name = args::get(name_arg);

    try {
        PegHeader header = read_headerfile(header_in_filename);
        read_datafile(data_in_filename, header);

        modify_texture(texture_name, header, new_name, flags);

        write_datafile(data_out_filename, header);
        write_headerfile(header_out_filename, header);
    } catch (const exit_error& e) {
        return e.status;
    }

    return 0;
}

void modify_texture(const std::string& texture_name, PegHeader& header,
    const std::string& new_name, uint16_t flags)
{
    size_t index = header.entry_index(texture_name);
    if (index == SIZE_MAX) {
        errormsg() << "Texture does not exist" << std::endl;
        throw exit_error(1);
    }

    PegEntry& entry = header.entries.at(index);
    if (!new_name.empty()) {
        entry.filename = new_name;
    }
    if (flags != 0xFFFF) {
        entry.flags = flags;
    }
}
