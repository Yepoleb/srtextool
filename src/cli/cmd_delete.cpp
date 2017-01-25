#include <stddef.h>
#include <string>
#include <vector>
#include <iostream>

#include "args.hxx"

#include "../headerfile.hpp"
#include "../path.hpp"
#include "../errors.hpp"
#include "shared.hpp"

void delete_textures(const std::vector<std::string>& texture_names, PegHeader& header);

static const char* HELP_DELETE =
R"(
Deletes textures from a container.

Usage: % [options] <header> <textures...>

Options:

  -h, --help                        Display this help menu
  -o [output], --output=[output]    Directory to write the new container to
  header                            Header file ending with cvbm_pc or cpeg_pc
  textures                          Textures to delete

)";

int cmd_delete(std::string progname,
    std::vector<std::string>::const_iterator beginargs,
    std::vector<std::string>::const_iterator endargs)
{
    progname += " d";
    args::ArgumentParser parser("");
    args::HelpFlag help(parser, "help", "", {'h', "help"});
    args::Positional<std::string> header_arg(parser, "header", "");
    args::PositionalList<std::string> textures_arg(parser, "textures", "");
    args::ValueFlag<std::string> output_arg(parser, "output", "", {'o', "output"});

    try {
        parser.ParseArgs(beginargs, endargs);
    } catch (args::Help) {
        std::cerr << help_format(HELP_DELETE, progname);
        return 0;
    } catch (const args::ParseError& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << help_format(HELP_DELETE, progname);
        return 1;
    } catch (const args::ValidationError& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    if (!header_arg) {
        std::cerr << help_format(HELP_DELETE, progname);
        return 1;
    }
    if (!textures_arg) {
        std::cerr << "[Error] Textures argument is missing" << std::endl;
        std::cerr << help_format(HELP_DELETE, progname);
        return 1;
    }

    std::string header_in_filename = args::get(header_arg);
    std::string data_in_filename = get_data_filename(header_in_filename);
    if (data_in_filename.empty()) {
        std::cerr << "[Error] Invalid file extension" << std::endl;
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

    std::vector<std::string> texture_names = args::get(textures_arg);

    try {
        PegHeader header = read_headerfile(header_in_filename);
        read_datafile(data_in_filename, header);

        delete_textures(texture_names, header);

        write_datafile(data_out_filename, header);
        write_headerfile(header_out_filename, header);
    } catch (const exit_error& e) {
        return e.status;
    }

    return 0;
}

void delete_textures(const std::vector<std::string>& texture_names, PegHeader& header)
{
    for (const std::string& name : texture_names) {
        header.remove_entry(name);
    }
}
