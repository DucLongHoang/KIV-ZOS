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
static constexpr uint SHORT_THRESHOLD = 5000;

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
        static void write_to_stream(std::iostream& stream, T& data) {
            uint streamSize = sizeof(T);
            stream.write(reinterpret_cast<char *>(&data), streamSize);
        }

        template<typename  T1, typename ... T>
        static void write_to_stream(std::iostream& stream, T1& data, T& ... args) {
            write_to_stream(stream, data);
            write_to_stream(stream, args ...);
        }

        template<typename T>
        static T read_from_stream(std::iostream &stream) {
            T data;
            stream.read(reinterpret_cast<char *>(&data), sizeof(T));
            return data;
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

        static std::string remove_padding(const std::string& str) {
            std::string result{str};
            result.erase(std::remove(result.begin(), result.end(), '\0'), result.end());
            return result;
        }

        static bool is_white_space(const std::string &str) {
            return std::all_of(str.begin(), str.end(), isspace);
        }
};