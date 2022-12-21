#pragma once

#include <string>
#include <vector>
#include <iomanip>
#include <sstream>
#include <iostream>

using uint = unsigned int;
using uchar = unsigned char;

static constexpr uchar FILENAME_LEN = 13;
static constexpr uchar SIGNATURE_LEN = 9;

// converting to bytes
static constexpr uint operator"" _B(unsigned long long int b) {
    return b;
}
static constexpr uint operator"" _KB(unsigned long long int kb) {
    return kb * 1024_B;
}
static constexpr uint operator"" _MB(unsigned long long int mb) {
    return mb * 1024_KB;
}

static constexpr uint CLUSTER_SIZE = 512_B;

/**
 * Structure Range - with lower and upper bound.
 * The bounds are inclusive.
 */
struct Range {
    uint lower;
    uint upper;
};

class Utils {
    public:
        template<typename ... T>
        static constexpr uint sum_sizeof(T ... args) {
            return (0 + ... + sizeof(args));
        }

        template<typename T>
        static void write_to_stream(std::iostream &stream, T &data) {
            uint streamSize = sizeof(T);
            stream.write(reinterpret_cast<char *>(&data), streamSize);
        }

        template<typename T>
        static void read_from_stream(std::iostream &stream, T &data) {
            uint streamSize = sizeof(T);
            stream.read(reinterpret_cast<char *>(&data), streamSize);
        }

        static void string_to_stream(std::iostream &stream, const std::string &string) {
            auto str = zero_padded_string(string, string.size());
            stream.write(str.c_str(), string.size());
        }

        static std::string string_from_stream(std::iostream &stream, uint streamSize) {
            char temp[streamSize];
            stream.read(temp, streamSize);
            return std::string{temp, streamSize};
        }

        static std::string zero_padded_string(const std::string &str, uint size) {
            std::ostringstream stream;
            stream << std::left << std::setw(size) << std::setfill('\0') << str ;
            return stream.str();

        }

        static bool is_white_space(const std::string &str) {
            return std::all_of(str.begin(), str.end(), isspace);
        }
};