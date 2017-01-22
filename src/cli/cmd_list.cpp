#include <string>
#include <vector>
#include <iostream>
#include <iomanip> // std::hex
#include <sstream> // std::stringstream

#include "args.hxx"

#include "../headerfile.hpp"
#include "../path.hpp"
#include "../errors.hpp"
#include "shared.hpp"

std::string list_header(const PegHeader& header);

static const char* HELP_LIST =
R"(
Lists header fields and texture entries.

Usage: % [options] <header>

Options:

  -h, --help                        Display this help menu
  header                            Header file ending with cvbm_pc or cpeg_pc

)";

int cmd_list(std::string progname,
    std::vector<std::string>::const_iterator beginargs,
    std::vector<std::string>::const_iterator endargs)
{
    progname += " l";
    args::ArgumentParser parser("");
    args::HelpFlag help(parser, "help", "", {'h', "help"});
    args::Positional<std::string> header_arg(parser, "header", "");

    try {
        parser.ParseArgs(beginargs, endargs);
    } catch (args::Help) {
        std::cerr << help_format(HELP_LIST, progname);
        return 0;
    } catch (const args::ParseError& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << help_format(HELP_LIST, progname);
        return 1;
    } catch (const args::ValidationError& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    if (!header_arg) {
        std::cerr << help_format(HELP_LIST, progname);
        return 1;
    }

    std::string header_filename = args::get(header_arg);

    try {
        PegHeader header = read_headerfile(header_filename);
        std::cout << list_header(header);
    } catch (const exit_error& e) {
        return e.status;
    }

    return 0;
}

std::string list_header(const PegHeader& header)
{
    std::stringstream stream;

    stream << "Version: " << header.version << std::endl;
    stream << "Platform: " << header.platform << std::endl;
    stream << "Dir block (cpeg) size: " << header.dir_block_size << std::endl;
    stream << "Data block (gpeg) size: " << header.data_block_size << std::endl;
    stream << "Bitmap count: " << header.num_bitmaps << std::endl;
    stream << "Entries count: " << header.total_entries << std::endl;
    stream << "Flags: 0x" << std::hex << header.flags << std::dec << std::endl;
    stream << "Alignment: " << header.alignment << std::endl;
    stream << std::endl;

    stream << "Entries: " << std::endl;
    for (const PegEntry& entry : header.entries) {
        stream << "Name: " << entry.filename << std::endl;
        stream << "Dimensions: " << entry.width << "x" << entry.height << std::endl;
        stream << "Format: " << get_format_name(entry.bm_fmt) << std::endl;
        if (entry.flags) {
            stream << "Flags: " << get_entry_flag_names(entry.flags) << std::endl;
        }
        if (entry.mip_levels > 1) {
            stream << "Mip levels: " << static_cast<int>(entry.mip_levels) << std::endl;
        }
        if (entry.flags & BM_F_ANIM_SHEET) {
            stream << "Animation dimensions: "
                << entry.anim_tiles_width << "x"
                << entry.anim_tiles_height << std::endl;
        }
        stream << "Offset: 0x" << std::hex << entry.offset << std::dec << std::endl;
        stream << "Texture size: " << entry.data_size << std::endl;
        stream << std::endl;
    }

    return stream.str();
}
