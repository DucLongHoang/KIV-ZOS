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

/**
 * Utils class - contains static utility methods for usage across all files.
 */
class Utils {
    public:
        /**
         * Method calculates the sum size of all arguments.
         * @tparam T types of arguments
         * @param args arguments
         * @return sum of all sizes of arguments
         */
        template<typename ... T>
        static constexpr uint sum_sizeof(T ... args) {
            return (0 + ... + sizeof(args));
        }

        /**
         * Method writes any data to a stream.
         * @tparam T type of data
         * @param stream to be written to
         * @param data to be written
         */
        template<typename T>
        static void write_to_stream(std::iostream& stream, T& data) {
            uint streamSize = sizeof(T);
            stream.write(reinterpret_cast<char *>(&data), streamSize);
        }

        /**
         * Variadic version of the previous method.
         * @tparam T1 type of the first template arg
         * @tparam T the rest of the template args
         * @param stream to be written to
         * @param data to be written
         * @param args also data to be written
         */
        template<typename  T1, typename ... T>
        static void write_to_stream(std::iostream& stream, T1& data, T& ... args) {
            write_to_stream(stream, data);
            write_to_stream(stream, args ...);
        }

        /**
         * Method reads any data from a stream.
         * @tparam T data type to be returned
         * @param stream to be read from
         * @return the data of type T that was read from the stream
         */
        template<typename T>
        static T read_from_stream(std::iostream &stream) {
            T data;
            stream.read(reinterpret_cast<char *>(&data), sizeof(T));
            return data;
        }

        /**
         * Method writes a string to a stream.
         * @param stream to be written to
         * @param string to be written
         */
        static void string_to_stream(std::iostream &stream, const std::string &string) {
            auto str = zero_padded_string(string, string.size());
            stream.write(str.c_str(), string.size());
        }

        /**
         * Method reads a string from a stream.
         * @param stream to be read from
         * @param streamSize size of string to be read
         * @return string of size streamSize from stream
         */
        static std::string string_from_stream(std::iostream &stream, uint streamSize) {
            char temp[streamSize];
            stream.read(temp, streamSize);
            return std::string{temp, streamSize};
        }

        /**
         * Method returns a zero padded version of string.
         * @param str string to be padded
         * @param size of the resulting string, pad included
         * @return
         */
        static std::string zero_padded_string(const std::string &str, uint size) {
            std::ostringstream stream;
            stream << std::left << std::setw(size) << std::setfill('\0') << str ;
            return stream.str();
        }

        /**
         * Method returns a padded string without padding.
         * @param str that was padded
         * @return string with no padding
         */
        static std::string remove_padding(const std::string& str) {
            std::string result{str};
            result.erase(std::remove(result.begin(), result.end(), '\0'), result.end());
            return result;
        }

        /**
         * Method checks if string is a whitespace or not.
         * @param str string to be checked for whitespace
         * @return true if string consists of only whitespaces, else false
         */
        static bool is_white_space(const std::string &str) {
            return std::all_of(str.begin(), str.end(), isspace);
        }
};