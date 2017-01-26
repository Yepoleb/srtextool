#include <stddef.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm> // std::find

#include "args.hxx"

#include "../headerfile.hpp"
#include "../ddsfile.hpp"
#include "../path.hpp"
#include "../errors.hpp"
#include "../gcc/abi_fix.hpp"
#include "shared.hpp"

void write_dds(const std::string& output_dir, const PegHeader& header,
    const std::vector<std::string>& texture_names);

static const char* HELP_EXTRACT =
R"(
Extracts the textures in a container as DDS files.

Usage: % [options] <header> [textures...]

Options:

  -h, --help                        Display this help menu
  -o [output], --output=[output]    Directory to write the files to
  header                            Header file ending with cvbm_pc or cpeg_pc
  textures                          Texture names if you only want to extract certain
                                    textures

)";

int cmd_extract(std::string progname,
    std::vector<std::string>::const_iterator beginargs,
    std::vector<std::string>::const_iterator endargs)
{
    progname += " x";
    args::ArgumentParser parser("");
    args::HelpFlag help(parser, "help", "", {'h', "help"});
    args::Positional<std::string> header_arg(parser, "header", "");
    args::PositionalList<std::string> textures_arg(parser, "textures", "");
    args::ValueFlag<std::string> output_arg(parser, "output", "", {'o', "output"});

    try {
        parser.ParseArgs(beginargs, endargs);
    } catch (args::Help) {
        std::cerr << help_format(HELP_EXTRACT, progname);
        return 0;
    } catch (const args::ParseError& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << help_format(HELP_EXTRACT, progname);
        return 1;
    } catch (const args::ValidationError& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    if (!header_arg) {
        std::cerr << help_format(HELP_EXTRACT, progname);
        return 1;
    }

    std::string header_filename = args::get(header_arg);
    std::string data_filename = get_data_filename(header_filename);
    if (data_filename.empty()) {
        std::cerr << "[Error] Invalid file extension." << std::endl;
        return 1;
    }

    std::string output_dir = args::get(output_arg);
    std::vector<std::string> texture_names = args::get(textures_arg);

    try {
        PegHeader header = read_headerfile(header_filename);
        read_datafile(data_filename, header);

        write_dds(output_dir, header, texture_names);
    } catch (const exit_error& e) {
        return e.status;
    }

    return 0;
}

void write_dds(const std::string& output_dir, const PegHeader& header,
    const std::vector<std::string>& texture_names)
{
    for (const PegEntry& entry : header.entries) {

        // Filter entries, skip if names are empty

        if (!texture_names.empty()) {
            auto found = std::find(texture_names.begin(), texture_names.end(), entry.filename);
            if (found == texture_names.end()) {
                continue;
            }
        }

        // Make dds filename

        std::string dds_filepath = entry.filename + ".dds";
        if (!output_dir.empty()) {
            // Use a custom output directory
            dds_filepath = path::join(output_dir, dds_filepath);
        }

        // Convert to DDS header

        DDSHeader ddsheader;
        try {
            ddsheader = entry.to_dds();
        } catch (const std::exception& e) {
            std::cerr << "[Error] Failed to convert entry: " << e.what() << std::endl;
            throw exit_error(1);
        }

        // Open DDS file

        std::ofstream ddsfile;
        set_ios_exceptions(ddsfile);
        try {
            GCC_ABI_WORKAROUND_START
            ddsfile.open(dds_filepath, OPENMODE_WRITE);
            GCC_ABI_WORKAROUND_END
        } catch (std::ios::failure) {
            std::cerr << "[Error] Failed to open DDS file for writing: " << dds_filepath << std::endl;
            throw exit_error(1);
        }

        // Write DDS file

        try {
            GCC_ABI_WORKAROUND_START
            ddsheader.write(ddsfile);
            ddsfile.write(entry.data.data(), entry.data.size());
            GCC_ABI_WORKAROUND_END
        } catch (std::ios::failure) {
            std::cerr << "[Error] Failed to write DDS file: " << get_stream_error(ddsfile) << std::endl;
            throw exit_error(1);
        } catch (const std::exception& e) {
            std::cerr << "[Error] Failed to write DDS file: " << e.what() << std::endl;
            throw exit_error(1);
        }
    }
}
