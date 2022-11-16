#include "Filesystem.hpp"

bool FAT::fs_creat(const std::vector<std::string> &args) {

    return false;
}

bool FAT::fs_open(const std::vector<std::string> &args) {

    return false;
}

bool FAT::fs_read(const std::vector<std::string> &args) {

    return false;
}

bool FAT::fs_write(const std::vector<std::string> &args) {
    std::for_each(args.begin(), args.end(), [this, args](const std::string& s) {
        fs << s << std::endl;
    });

    return false;
}

bool FAT::fs_close(const std::vector<std::string> &args) {

    return false;
}
