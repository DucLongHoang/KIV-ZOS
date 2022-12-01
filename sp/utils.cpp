#include <iomanip>
#include <sstream>
#include "utils.hpp"

void write_to_stream(std::fstream &stream, const std::string &string, unsigned int streamSize) {
    auto str = zero_padded_string(string, streamSize);
    stream.write(str.c_str(), streamSize);
}

void read_from_stream(std::fstream &stream, std::string &string, unsigned int streamSize) {
    char temp[streamSize];
    stream.read(temp, streamSize);
    string = std::string(temp, streamSize);
}

std::string zero_padded_string(const std::string& str, unsigned char size) {
    std::ostringstream stream;
    stream << std::left << std::setw(size) << std::setfill('\0') << str ;
    return stream.str();
}

bool is_white_space(const std::string& str) {
    return std::all_of(str.begin(), str.end(), isspace);
}