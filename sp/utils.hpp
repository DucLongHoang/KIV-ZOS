#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <concepts>

using uint = unsigned int;

constexpr uint FILENAME_LEN = 13;
constexpr uint SIGNATURE_LEN = 9;

// converting to bytes
constexpr uint operator"" _B(unsigned long long int b) {
    return b;
}
constexpr uint operator"" _KB(unsigned long long int kb) {
    return kb * 1024_B;
}
constexpr uint operator"" _MB(unsigned long long int mb) {
    return mb * 1024_KB;
}
constexpr uint CLUSTER_SIZE = 512_B;

template<typename ... Targs>
constexpr uint sum_sizeof(Targs ... targs) {
    return (0 + ... + sizeof(targs));
}

template<typename T>
void write_to_stream(std::iostream &stream, T &data) {
    uint streamSize = sizeof(T);
    stream.write(reinterpret_cast<char *>(&data), streamSize);
}

template<typename T>
void read_from_stream(std::iostream &stream, T &data) {
    uint streamSize = sizeof(T);
    stream.read(reinterpret_cast<char *>(&data), streamSize);
}

void string_to_stream(std::iostream &stream, const std::string &string);

std::string string_from_stream(std::iostream &stream, uint streamSize);

std::string zero_padded_string(const std::string &str, uint size);

bool is_white_space(const std::string &str);
