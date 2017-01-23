#include <stdint.h>
#include <stddef.h>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <exception>

#include "../headerfile.hpp"
#include "../path.hpp"
#include "../errors.hpp"
#include "shared.hpp"

std::string get_data_filename(const std::string& header_filename)
{
    std::string ext = path::extension(header_filename);
    std::string root = path::remove_extension(header_filename);
    if (ext == "cvbm_pc") {
        return root + ".gvbm_pc";
    } else if (ext == "cpeg_pc") {
        return root + ".gpeg_pc";
    } else {
        return "";
    }
}

const char* get_stream_error(const std::ios& stream)
{
    if (stream.eof()) {
        return "End of file";
    } else if (stream.bad()) {
        return "I/O error";
    } else if (stream.fail()) {
        return "Unknown error";
    } else {
        return "No error";
    }
}

void align(std::ostream& stream, std::streamoff alignment)
{
    std::streampos current_pos = stream.tellp();
    std::streamoff to_align = current_pos % alignment;
    if (to_align > 0) {
        stream.seekp(to_align, std::ios::cur);
    }
}



PegHeader read_headerfile(const std::string& filename)
{
    // Open header file

    std::ifstream headerfile;
    set_ios_exceptions(headerfile);
    try {
        headerfile.open(filename, OPENMODE_READ);
    } catch (std::ios::failure) {
        std::cerr << "[Error] Failed to open header file: " << filename << std::endl;
        throw exit_error(1);
    }

    // Read header

    PegHeader header;
    try {
        header.read(headerfile);
        headerfile.close();
    } catch (std::ios::failure) {
        std::cerr << "[Error] Failed to read header: " << get_stream_error(headerfile) << std::endl;
        throw exit_error(1);
    } catch (const std::exception& e) {
        std::cerr << "[Error] Failed to read header: " << e.what() << std::endl;
        throw exit_error(1);
    }

    return header;
}

void write_headerfile(const std::string& filename, PegHeader& header)
{
    // Open header file

    std::ofstream headerfile;
    set_ios_exceptions(headerfile);
    try {
        headerfile.open(filename, OPENMODE_WRITE);
    } catch (std::ios::failure) {
        std::cerr << "[Error] Failed to open header file for writing: " << filename << std::endl;
        throw exit_error(1);
    }

    // Update header size

    header.dir_block_size = static_cast<uint32_t>(header.size());

    // Write header

    try {
        header.write(headerfile);
        headerfile.close();
    } catch (std::ios::failure) {
        std::cerr << "[Error] Failed to write header: " << get_stream_error(headerfile) << std::endl;
        throw exit_error(1);
    } catch (const std::exception& e) {
        std::cerr << "[Error] Failed to write header: " << e.what() << std::endl;
        throw exit_error(1);
    }
}

void read_datafile(const std::string& filename, PegHeader& header)
{
    if (header.total_entries == 0) {
        return;
    }

    // Open data file

    std::ifstream datafile;
    set_ios_exceptions(datafile);
    try {
        datafile.open(filename, OPENMODE_READ);
    } catch (std::ios::failure) {
        std::cerr << "[Error] Failed to open data file: " << filename << std::endl;
        throw exit_error(1);
    }

    for (PegEntry& entry : header.entries) {

        // Read texture data

        std::vector<char> texture_data(entry.data_size);
        try {
            datafile.seekg(entry.offset);
            datafile.read(texture_data.data(), entry.data_size);
        } catch (std::ios::failure) {
            std::cerr << "[Error] Failed to read texture data: " << get_stream_error(datafile) << std::endl;
            throw exit_error(1);
        }
        entry.data = std::move(texture_data);
    }
}

void write_datafile(const std::string& filename, PegHeader& header)
{
    // Open data file

    std::ofstream datafile;
    set_ios_exceptions(datafile);
    try {
        datafile.open(filename, OPENMODE_WRITE);
    } catch (std::ios::failure) {
        std::cerr << "[Error] Failed to open data file for writing: " << filename << std::endl;
        throw exit_error(1);
    }

    for (PegEntry& entry : header.entries) {

        // Write entry

        try {
            align(datafile, header.alignment);
            entry.offset = datafile.tellp();
            datafile.write(entry.data.data(), entry.data.size());
        } catch (std::ios::failure) {
            std::cerr << "[Error] Failed to write data file: " << get_stream_error(datafile) << std::endl;
            throw exit_error(1);
        }
    }

    header.data_block_size = static_cast<uint32_t>(datafile.tellp());
    datafile.close();
}
