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
#include "../common.hpp"
#include "../gcc/abi_fix.hpp"
#include "shared.hpp"

void update_files(const std::vector<std::string>& dds_filenames, PegHeader& header);
size_t get_file_size(std::istream& stream);

const size_t FOURCC_SIZE = 4;

static const char* HELP_ADD =
R"(
Adds textures to a container or updates them if they already exist.

Usage: % [options] <header> [files...]

Options:

  -h, --help                        Display this help menu
  -o [output], --output=[output]    Directory to write the new container to
  -i [input], --input=[input]       Directory to update all existing textures
                                    from
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
    args::ValueFlag<std::string> input_arg(parser, "input", "", {'i', "input"});

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
    if (!(files_arg || input_arg)) {
        errormsg() << "Files or input argument is missing" << std::endl;
        std::cerr << help_format(HELP_ADD, progname);
        return 1;
    }
    if (files_arg && input_arg) {
        errormsg() << "Can't use files and input argument at the same time" << std::endl;
        std::cerr << help_format(HELP_ADD, progname);
        return 1;
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

    PegHeader header;

    if (path::exists(header_in_filename)) {
        try {
            header = read_headerfile(header_in_filename);
            read_datafile(data_in_filename, header);
        } catch (const exit_error& e) {
            return e.status;
        }
    } else {
        infomsg() << "Input file does not exist, creating a new one" << std::endl;
    }

    std::vector<std::string> dds_filenames;
    if (files_arg) {
        dds_filenames = args::get(files_arg);
    } else {
        std::string input_dir = args::get(input_arg);
        for (const PegEntry& entry : header.entries) {
            std::string filename = path::join(input_dir, entry.filename + ".dds");
            dds_filenames.push_back(filename);
        }
    }

    try {
        update_files(dds_filenames, header);

        write_datafile(data_out_filename, header);
        write_headerfile(header_out_filename, header);
    } catch (const exit_error& e) {
        return e.status;
    }

    return 0;
}

void update_files(const std::vector<std::string>& dds_filenames, PegHeader& header)
{
    for (const std::string& dds_filename : dds_filenames) {

        std::string texture_name = path::remove_extension(path::basename(dds_filename));

        PegEntry entry;

        // Check if entry with the same name already exists

        size_t existing_index = header.entry_index(texture_name);
        bool is_new = (existing_index == SIZE_MAX);
        if (is_new) {
            entry.filename = texture_name;
            infomsg() << "Adding " << entry.filename << std::endl;
        } else {
            entry = header.entries.at(existing_index);
            infomsg() << "Updating " << entry.filename << std::endl;
        }

        // Open DDS file

        std::ifstream ddsfile;
        set_ios_exceptions(ddsfile);
        try {
            GCC_ABI_WORKAROUND_START
            ddsfile.open(dds_filename, OPENMODE_READ);
            GCC_ABI_WORKAROUND_END
        } catch (std::ios::failure) {
            errormsg() << "Failed to open DDS file: " << dds_filename << std::endl;
            throw exit_error(1);
        }

        // Read header and texture data

        DDSHeader dds_header;
        try {
            GCC_ABI_WORKAROUND_START
            dds_header.read(ddsfile);

            size_t data_size = get_file_size(ddsfile) - DDS_HEADER_SIZE - FOURCC_SIZE;
            entry.data_size = static_cast<uint32_t>(data_size);
            entry.data.resize(data_size);
            ddsfile.read(entry.data.data(), data_size);
            ddsfile.close();
            GCC_ABI_WORKAROUND_END
        } catch (std::ios::failure) {
            errormsg() << "Failed to read DDS file: " << get_stream_error(ddsfile) << std::endl;
            throw exit_error(1);
        } catch (const std::exception& e) {
            errormsg() << "Failed to read DDS file: " << e.what() << std::endl;
            throw exit_error(1);
        }

        // Detect format change

        if (!is_new) {
            TextureFormat dds_format = detect_pixelformat(dds_header.ddspf);
            if (dds_format != entry.bm_fmt) {
                warnmsg() << "New texture format doesn't match previous format" << std::endl;
                warnmsg() << "Switching from " << get_format_name(entry.bm_fmt) <<
                    " to " << get_format_name(dds_format) << std::endl;
            }

            if ((dds_header.width != entry.width) || (dds_header.height != entry.height)) {
                warnmsg() << "Changing dimensions from " <<
                    entry.width << "x" << entry.height << " to " <<
                    dds_header.width << "x" << dds_header.height << std::endl;
            }

            if (dds_header.mipmap_count != entry.mip_levels) {
                warnmsg() << "Changing mip level from " <<
                    static_cast<int>(entry.mip_levels) << " to " <<
                    dds_header.mipmap_count << std::endl;
            }
        }

        // Convert header

        try {
            entry.update_dds(dds_header);
        } catch (const std::exception& e) {
            errormsg() << "Failed to convert DDS to PEG header: " << e.what() << std::endl;
            throw exit_error(1);
        }

        // Insert new header and texture

        if (is_new) {
            header.add_entry(std::move(entry));
        } else {
            header.entries.at(existing_index) = std::move(entry);
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
