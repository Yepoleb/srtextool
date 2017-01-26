#define _GLIBCXX_USE_CXX11_ABI 1
#include <ios>
#include <typeinfo>
#include <exception>

extern const std::type_info& old_abi_failure;
const std::type_info& new_abi_failure = typeid(std::ios::failure);

bool is_ios_failure(const std::exception& e) {
    const std::type_info& e_type = typeid(e);
    if (e_type.hash_code() == old_abi_failure.hash_code()) {
        return true;
    } else if (e_type.hash_code() == new_abi_failure.hash_code()) {
        return true;
    } else {
        return false;
    }
}
