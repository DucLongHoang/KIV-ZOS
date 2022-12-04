#include <iomanip>
#include <sstream>
#include "utils.hpp"

void string_to_stream(std::fstream &stream, std::string &string) {
    auto str = zero_padded_string(string, string.size());
    stream.write(str.c_str(), string.size());
}

std::string string_from_stream(std::fstream &stream, unsigned int streamSize) {
    char temp[streamSize];
    stream.read(temp, streamSize);
    return std::string{temp, streamSize};
}

std::string zero_padded_string(const std::string& str, unsigned int size) {
    std::ostringstream stream;
    stream << std::left << std::setw(size) << std::setfill('\0') << str ;
    return stream.str();
}

bool is_white_space(const std::string& str) {
    return std::all_of(str.begin(), str.end(), isspace);
}