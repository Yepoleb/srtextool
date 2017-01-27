#pragma once
#include <iostream>

#define MAKEFOURCC(a, b, c, d) ((d << 24) | (c << 16) | (b << 8) | (a << 0))
#define ARRAYSIZE(a) (sizeof(a) / sizeof(a[0]))

inline std::ostream& errormsg()
{
    return std::cerr << "[Error] ";
}

inline std::ostream& warnmsg()
{
    return std::cerr << "[Warning] ";
}

inline std::ostream& infomsg()
{
    return std::cerr << "[Info] ";
}
