#include <stdint.h>
#include <stddef.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include "args.hxx"

#include "../headerfile.hpp"
#include "../ddsfile.hpp"
#include "../path.hpp"
#include "../errors.hpp"
#include "shared.hpp"

void update_files(const std::vector<std::string>& dds_filenames,
    PegHeader& header, tex_vector_t& textures);
size_t get_file_size(std::istream& stream);

const size_t FOURCC_SIZE = 4;

static const char* HELP_ADD =
R"(
Adds textures to a container or updates them if they already exist.

Usage: % [options] <header> <files...>

Options:

  -h, --help                        Display this help menu
  -o [output], --output=[output]    Directory to write the new container to
  header                            Header file ending with cvbm_pc or cpeg_pc
  files                             Files to add or update

)";

int cmd_add(std::string progname,
    std::vector<std::string>::const_iterator beginargs,
    std::vector<std::string>::const_iterator endargs)
{
    progname += " a";
    args::ArgumentParser parser("");
    args::HelpFlag help(parser, "help", "", {'h', "help"});
    args::Positional<std::string> header_arg(parser, "header", "");
    args::PositionalList<std::string> files_arg(parser, "files", "");
    args::ValueFlag<std::string> output_arg(parser, "output", "", {'o', "output"});

    try {
        parser.ParseArgs(beginargs, endargs);
    } catch (args::Help) {
        std::cerr << help_format(HELP_ADD, progname);
        return 0;
    } catch (const args::ParseError& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << help_format(HELP_ADD, progname);
        return 1;
    } catch (const args::ValidationError& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    if (!header_arg) {
        std::cerr << help_format(HELP_ADD, progname);
        return 1;
    }
    if (!files_arg) {
        std::cerr << "[Error] Files argument is missing" << std::endl;
        std::cerr << parser;
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

    std::vector<std::string> dds_filenames = args::get(files_arg);

    try {
        PegHeader header = read_headerfile(header_in_filename);
        tex_vector_t textures = read_datafile(data_in_filename, header);

        update_files(dds_filenames, header, textures);

        write_datafile(data_out_filename, header, textures);
        write_headerfile(header_out_filename, header);
    } catch (const exit_error& e) {
        return e.status;
    }

    return 0;
}

void update_files(const std::vector<std::string>& dds_filenames,
    PegHeader& header, tex_vector_t& textures)
{
    for (const std::string& dds_filename : dds_filenames) {

        std::string texture_name = path::remove_extension(path::basename(dds_filename));

        PegEntry entry;

        // Check if entry with the same name already exists

        size_t existing_index = header.entry_index(texture_name);
        if (existing_index != SIZE_MAX) {
            entry = header.entries.at(existing_index);
        } else {
            entry.filename = texture_name;
        }

        // Open DDS file

        std::ifstream ddsfile;
        set_ios_exceptions(ddsfile);
        try {
            ddsfile.open(dds_filename, OPENMODE_READ);
        } catch (const std::ios::failure&) {
            std::cerr << "[Error] Failed to open DDS file: " << dds_filename << std::endl;
            throw exit_error(1);
        }

        // Read header and texture data

        DDSHeader ddsheader;
        size_t texture_size;
        std::vector<char> texture_data;
        try {
            ddsheader.read(ddsfile);
            texture_size = get_file_size(ddsfile) - DDS_HEADER_SIZE - FOURCC_SIZE;
            texture_data.resize(texture_size);
            ddsfile.read(texture_data.data(), texture_size);
            ddsfile.close();
        } catch (const std::ios::failure&) {
            std::cerr << "[Error] Failed to read DDS file: " << get_stream_error(ddsfile) << std::endl;
            throw exit_error(1);
        } catch (const std::exception& e) {
            std::cerr << "[Error] Failed to read DDS file: " << e.what() << std::endl;
            throw exit_error(1);
        }

        // Convert header

        try {
            entry.update_dds(ddsheader);
        } catch (const std::exception& e) {
            std::cerr << "[Error] Failed to convert DDS to PEG header: " << e.what() << std::endl;
            throw exit_error(1);
        }
        entry.data_size = static_cast<uint32_t>(texture_size);

        // Insert new header and texture

        if (existing_index != SIZE_MAX) {
            header.entries.at(existing_index) = std::move(entry);
            textures.at(existing_index) = std::move(texture_data);
        } else {
            header.add_entry(entry);
            textures.push_back(std::move(texture_data));
        }
    }
}

size_t get_file_size(std::istream& stream)
{
    size_t current_pos;
    current_pos = stream.tellg();
    stream.seekg(0, std::ios::end);
    size_t end_pos = stream.tellg();
    stream.seekg(current_pos);

    return end_pos;
}