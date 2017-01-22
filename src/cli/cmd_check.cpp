#include <stddef.h>
#include <string>
#include <vector>
#include <iostream>

#include "args.hxx"

#include "../headerfile.hpp"
#include "../errors.hpp"
#include "shared.hpp"

bool check_textures(const PegHeader& header, const tex_vector_t& textures);

static const char* HELP_CHECK =
R"(
Checks a texture file for errors. No output means the file is probably ok.

Usage: % [options] <header...>

Options:

  -h, --help                        Display this help menu
  header                            Header files ending with cvbm_pc or cpeg_pc

)";

int cmd_check(std::string progname,
    std::vector<std::string>::const_iterator beginargs,
    std::vector<std::string>::const_iterator endargs)
{
    progname += " c";
    args::ArgumentParser parser("");
    args::HelpFlag help(parser, "help", "", {'h', "help"});
    args::PositionalList<std::string> header_arg(parser, "header", "");

    try {
        parser.ParseArgs(beginargs, endargs);
    } catch (args::Help) {
        std::cerr << help_format(HELP_CHECK, progname);
        return 0;
    } catch (const args::ParseError& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << help_format(HELP_CHECK, progname);
        return 1;
    } catch (const args::ValidationError& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    if (!header_arg) {
        std::cerr << help_format(HELP_CHECK, progname);
        return 1;
    }

    std::vector<std::string> header_files = args::get(header_arg);
    for (const std::string& header_filename : header_files) {
        std::string data_filename = get_data_filename(header_filename);
        if (data_filename.empty()) {
            std::cerr << "[Error] Invalid file extension." << std::endl;
            continue;
        }

        try {
            PegHeader header = read_headerfile(header_filename);
            tex_vector_t textures = read_datafile(data_filename, header);

            bool failed = check_textures(header, textures);
            if (failed) {
                std::cout << "Failed: " << header_filename << std::endl;
            }
        } catch (const exit_error& e) {
            return e.status;
        }
    }

    std::cout << "Checked " << header_files.size() << " files" << std::endl;

    return 0;
}

#define CHECK_FIELD(cond) \
    if (!(cond)) { \
        std::cout << "Failed check: " << #cond << std::endl; \
        failed = true; \
    }

bool check_textures(const PegHeader& header, const tex_vector_t& textures)
{
    bool failed = false;

    CHECK_FIELD(header.dir_block_size == header.size());
    size_t combined_size = 0;
    for (const std::vector<char>& tex : textures) {
        combined_size += tex.size();
    }
    CHECK_FIELD(header.data_block_size >= combined_size);
    CHECK_FIELD(header.data_block_size <= combined_size + textures.size() * header.alignment);
    //CHECK_FIELD(header.num_bitmaps > 0);
    CHECK_FIELD(header.num_bitmaps == textures.size());
    CHECK_FIELD(header.num_bitmaps == header.total_entries);
    //CHECK_FIELD(header.total_entries > 0)
    CHECK_FIELD(header.flags == 0);
    CHECK_FIELD(header.alignment == 16);

    for (size_t entry_i = 0; entry_i < header.total_entries; entry_i++) {
        const PegEntry& entry = header.entries.at(entry_i);
        CHECK_FIELD(entry.offset < header.data_block_size);
        CHECK_FIELD(entry.offset + entry.data_size <= header.data_block_size);
        CHECK_FIELD(entry.width > 0);
        CHECK_FIELD(entry.height > 0);
        CHECK_FIELD(TextureFormat::PC_DXT1 <= entry.bm_fmt);
        CHECK_FIELD(TextureFormat::PC_A8 >= entry.bm_fmt);
        CHECK_FIELD(entry.pal_fmt == 0);
        CHECK_FIELD(entry.num_frames == 1);
        CHECK_FIELD(!entry.filename.empty());
        CHECK_FIELD(entry.pal_size == 0);
        CHECK_FIELD(entry.fps == 1);
        CHECK_FIELD(entry.mip_levels >= 1);
        CHECK_FIELD(entry.data_size >= 0);

        if (entry.flags & BM_F_ALPHA) {
            failed = true;
            std::cerr << "Alpha tex " << get_format_name(entry.bm_fmt) << std::endl;
        }
    }

    if (header.total_entries > 0) {
        const PegEntry& last_entry = header.entries.back();
        CHECK_FIELD(header.data_block_size <= last_entry.offset + last_entry.data_size + header.alignment);
    }

    return failed;
}
