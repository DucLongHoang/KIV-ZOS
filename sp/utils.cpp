#include <iomanip>
#include "utils.hpp"

extern std::string zero_padded_string(const std::string& str, unsigned char padSize) {
    std::ostringstream stream;
    stream << std::left << std::setw(padSize) << std::setfill('\0') << str ;
    return stream.str();
}

extern bool is_white_space(const std::string& str) {
    return std::all_of(str.begin(), str.end(), isspace);
}