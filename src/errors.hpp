#pragma once
#include <stddef.h>
#include <exception>
#include <stdexcept>

class field_error : public std::runtime_error
{
public:
    explicit field_error(const std::string& field_name_, const std::string& field_value_);

    std::string field_name;
    std::string field_value;
};

inline field_error::field_error(const std::string& field_name_, const std::string& field_value_)
    : std::runtime_error("Invalid value in field " + field_name_ + " (" + field_value_ + ")")
{
    field_name = field_name_;
    field_value = field_value_;
}



// Used for terminating the program inside shared functions

class exit_error : public std::exception
{
public:
    explicit exit_error(int status);
    virtual const char* what() const noexcept;

    int status;
};

inline exit_error::exit_error(int status_)
{
    status = status_;
}

inline const char* exit_error::what() const noexcept
{
    return "Program terminated";
}
