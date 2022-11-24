#include <algorithm>
#include "FAT.hpp"

FAT_Filesystem::FAT_Filesystem(const std::string &name, unsigned int diskSize) {


    if (std::filesystem::exists(name)) {
        mFS.open(name, std::ios::in | std::ios::out | std::ios::binary | std::ios::ate);
    }
    else {
        mFS.open(name, std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
    }
}

bool FAT_Filesystem::fs_creat(const std::vector<std::string> &args) {

    return false;
}

bool FAT_Filesystem::fs_open(const std::vector<std::string> &args) {

    return false;
}

bool FAT_Filesystem::fs_read(const std::vector<std::string> &args) {

    return false;
}

bool FAT_Filesystem::fs_write(const std::vector<std::string> &args) {
    std::for_each(args.begin(), args.end(), [this, args](const std::string& s) {
        mFS.write(s.c_str(), 1024);
    });

    return false;
}

bool FAT_Filesystem::fs_close(const std::vector<std::string> &args) {

    return false;
}
