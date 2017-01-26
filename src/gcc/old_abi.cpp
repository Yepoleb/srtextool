#define _GLIBCXX_USE_CXX11_ABI 0
#include <ios>
#include <typeinfo>

const std::type_info& old_abi_failure = typeid(std::ios::failure);
