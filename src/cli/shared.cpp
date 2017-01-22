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
#include "../byteio.hpp"
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



PegHeader read_headerfile(const std::string& filename)
{
    // Open header file

    std::ifstream headerfile;
    set_ios_exceptions(headerfile);
    try {
        headerfile.open(filename, OPENMODE_READ);
    } catch (const std::ios::failure&) {
        std::cerr << "[Error] Failed to open header file: " << filename << std::endl;
        throw exit_error(1);
    }

    // Read header

    PegHeader header;
    try {
        header.read(headerfile);
        headerfile.close();
    } catch (const std::ios::failure&) {
        std::cerr << "[Error] Failed to read header: " << get_stream_error(headerfile) << std::endl;
        throw exit_error(1);
    } catch (const std::exception& e) {
        std::cerr << "[Error] Failed to read header: " << e.what() << std::endl;
        throw exit_error(1);
    }

    return header;
}

tex_vector_t read_datafile(const std::string& filename, const PegHeader& header)
{
    tex_vector_t textures;

    if (header.total_entries == 0) {
        return textures;
    }

    // Open data file

    std::ifstream datafile;
    set_ios_exceptions(datafile);
    try {
        datafile.open(filename, OPENMODE_READ);
    } catch (const std::ios::failure&) {
        std::cerr << "[Error] Failed to open data file: " << filename << std::endl;
        throw exit_error(1);
    }

    for (const PegEntry& entry : header.entries) {

        // Read texture data

        std::vector<char> texture_data(entry.data_size);
        try {
            datafile.seekg(entry.offset);
            datafile.read(texture_data.data(), entry.data_size);
        } catch (const std::ios::failure&) {
            std::cerr << "[Error] Failed to read texture data: " << get_stream_error(datafile) << std::endl;
            throw exit_error(1);
        }
        textures.push_back(std::move(texture_data));
    }

    return textures;
}

void write_datafile(const std::string& filename, PegHeader& header,
    tex_vector_t& textures)
{
    // Open data file

    std::ofstream datafile;
    set_ios_exceptions(datafile);
    try {
        datafile.open(filename, OPENMODE_WRITE);
    } catch (const std::ios::failure&) {
        std::cerr << "[Error] Failed to open data file for writing: " << filename << std::endl;
        throw exit_error(1);
    }

    ByteWriter datafile_writer(datafile);

    for (size_t entry_i = 0; entry_i < header.total_entries; entry_i++) {

        PegEntry& entry = header.entries.at(entry_i);
        const std::vector<char>& texture_data = textures.at(entry_i);

        // Write entry

        try {
            datafile_writer.align(header.alignment);
            entry.offset = datafile_writer.tell();
            datafile_writer.write(texture_data.data(), texture_data.size());
        } catch (const std::ios::failure&) {
            std::cerr << "[Error] Failed to write data file: " << get_stream_error(datafile) << std::endl;
            throw exit_error(1);
        }
    }

    header.data_block_size = static_cast<uint32_t>(datafile.tellp());
    datafile.close();
}

void write_headerfile(const std::string& filename, PegHeader& header)
{
    // Open header file

    std::ofstream headerfile;
    set_ios_exceptions(headerfile);
    try {
        headerfile.open(filename, OPENMODE_WRITE);
    } catch (const std::ios::failure&) {
        std::cerr << "[Error] Failed to open header file for writing: " << filename << std::endl;
        throw exit_error(1);
    }

    // Update header size

    header.dir_block_size = static_cast<uint32_t>(header.size());

    // Write header

    try {
        header.write(headerfile);
        headerfile.close();
    } catch (const std::ios::failure&) {
        std::cerr << "[Error] Failed to write header: " << get_stream_error(headerfile) << std::endl;
        throw exit_error(1);
    } catch (const std::exception& e) {
        std::cerr << "[Error] Failed to write header: " << e.what() << std::endl;
        throw exit_error(1);
    }
}
