#include <algorithm>
#include "Filesystem.hpp"

FAT::FAT(const std::string &name) {


    if (std::filesystem::exists(name)) {
        mFS.open(name, std::ios::in | std::ios::out | std::ios::binary | std::ios::ate);
    }
    else {
        mFS.open(name, std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
    }
}

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
        mFS << s << std::endl;
    });

    return false;
}

bool FAT::fs_close(const std::vector<std::string> &args) {

    return false;
}
