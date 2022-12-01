#pragma once

#include <string>
#include <fstream>

// converting to bytes
constexpr unsigned int operator"" _KB(unsigned long long int kb) {
    return kb * 1024;
}
constexpr unsigned int operator"" _MB(unsigned long long int mb) {
    return mb * 1024_KB;
}
constexpr unsigned int CLUSTER_SIZE = 512_KB;

template<typename ... Targs>
constexpr unsigned int sum_sizeof(Targs ... targs) {
    return (0 + ... + sizeof(targs));
}

template<typename T>
void write_to_stream(std::fstream &stream, T &data, int streamSize = sizeof(T)) {
    stream.write(reinterpret_cast<char *>(&data), streamSize);
}

template<typename T>
void read_from_stream(std::fstream &stream, T &data, int streamSize = sizeof(T)) {
    stream.read(reinterpret_cast<char *>(&data), streamSize);
}

void write_to_stream(std::fstream &stream, const std::string &string, unsigned int streamSize);
void read_from_stream(std::fstream &stream, std::string &string, unsigned int streamSize);

std::string zero_padded_string(const std::string &str, unsigned char size);
bool is_white_space(const std::string &str);
