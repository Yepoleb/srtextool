#pragma once
#include <stddef.h>
#include <string>

namespace path
{

#ifdef _WIN32
static const char PRIMARY_SEPARATOR = '\\';
static const char* PATH_SEPARATORS = "/\\";
#else
static const char PRIMARY_SEPARATOR = '/';
static const char* PATH_SEPARATORS = "/";
#endif

inline std::string lstrip(const std::string& str, const std::string& chars)
{
    size_t first_valid = str.find_first_not_of(chars);
    return str.substr(first_valid, std::string::npos);
}

inline std::string rstrip(const std::string& str, const std::string& chars)
{
    size_t last_valid = str.find_last_not_of(chars);
    return str.substr(0, last_valid + 1);
}

inline std::string dirname(const std::string& filepath)
{
    size_t last_sep = filepath.find_last_of(PATH_SEPARATORS);
    if (last_sep == std::string::npos) {
        return "";
    }
    std::string dir_path = filepath.substr(0, last_sep);
    return rstrip(dir_path, PATH_SEPARATORS);
}

inline std::string basename(const std::string& filepath)
{
    size_t last_sep = filepath.find_last_of(PATH_SEPARATORS);
    return filepath.substr(last_sep + 1, std::string::npos);
}

inline std::string remove_extension(const std::string& filename)
{
    size_t last_dot = filename.find_last_of('.');
    return filename.substr(0, last_dot);
}

inline std::string extension(const std::string& filename)
{
    size_t last_dot = filename.find_last_of('.');
    if (last_dot == std::string::npos) {
        return "";
    }
    return filename.substr(last_dot + 1, std::string::npos);
}

inline std::string join(const std::string& path1, const std::string& path2)
{
    std::string path1_strip = rstrip(path1, PATH_SEPARATORS);
    std::string path2_strip = rstrip(path2, PATH_SEPARATORS);
    return path1_strip + PRIMARY_SEPARATOR + path2_strip;
}

} // namespace path
