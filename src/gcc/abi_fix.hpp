#pragma once
#include <exception>
#include <ios>

// Workaround for catching std::ios::failure that have been thrown by libstdc++
// using a different ABI version. Works by having two std::type_info
// references, one for each version and comparing the exception against them.
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66145

#ifdef GCC_ABI_WORKAROUND_ENABLED

bool is_ios_failure(const std::exception& e);

#define GCC_ABI_WORKAROUND_START try {
#define GCC_ABI_WORKAROUND_END \
    } catch (std::exception& e) { \
        if (is_ios_failure(e)) { \
            throw std::ios::failure(""); \
        } else { \
            throw e; \
        } \
    }

#else

#define GCC_ABI_WORKAROUND_START
#define GCC_ABI_WORKAROUND_END

#endif
