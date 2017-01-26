#pragma once
#include <stddef.h>
#include <string>
#include <vector>
#include <ios>
#include <functional>

struct PegHeader;

const std::ios::openmode OPENMODE_READ = std::ios::in | std::ios::binary;
const std::ios::openmode OPENMODE_WRITE = std::ios::out | std::ios::binary | std::ios::trunc;

using commandtype = std::function<int(const std::string&, std::vector<std::string>::const_iterator, std::vector<std::string>::const_iterator)>;

const char* get_stream_error(const std::ios& stream);
std::string get_data_filename(const std::string& header_filename);
void align(std::ostream& stream, std::streamoff alignment);

PegHeader read_headerfile(const std::string& filename);
void write_headerfile(const std::string& filename, PegHeader& header);
void read_datafile(const std::string& filename, PegHeader& header);
void write_datafile(const std::string& filename, PegHeader& header);

// Defined in cmd_*.cpp files

int cmd_add(std::string progname, std::vector<std::string>::const_iterator beginargs, std::vector<std::string>::const_iterator endargs);
int cmd_check(std::string progname, std::vector<std::string>::const_iterator beginargs, std::vector<std::string>::const_iterator endargs);
int cmd_delete(std::string progname, std::vector<std::string>::const_iterator beginargs, std::vector<std::string>::const_iterator endargs);
int cmd_extract(std::string progname, std::vector<std::string>::const_iterator beginargs, std::vector<std::string>::const_iterator endargs);
int cmd_list(std::string progname, std::vector<std::string>::const_iterator beginargs, std::vector<std::string>::const_iterator endargs);

inline void set_ios_exceptions(std::ios& stream)
{
    stream.exceptions(std::ios::badbit | std::ios::failbit);
}

inline std::string help_format(std::string help_str, const std::string& progname)
{
    return help_str.replace(help_str.find('%'), 1, progname);
}
