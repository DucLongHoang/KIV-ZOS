#pragma once

#include <string>
#include <sstream>

template<typename ... Targs>
constexpr unsigned int sum_sizeof(Targs ... targs) {
    return (0 + ... + sizeof(targs));
}

extern std::string zero_padded_string(const std::string& str, unsigned char padSize);
extern bool is_white_space(const std::string& str);