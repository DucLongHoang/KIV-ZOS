#pragma once

#include <array>
#include <fstream>
#include <utility>
#include <iostream>
#include <filesystem>

#include "IFilesystem.hpp"
#include "utils.hpp"

static const unsigned int CLUSTER_SIZE = 512 * 8;   // 512 bytes

class BootSector {
    public:
        std::array<char,9> mSignature;   // author login
        int mDiskSize;                   // total size of FS
        int mClusterSize;                // size of one cluster
        int mClusterCount;               // total number of clusters
        int mFatEntryCount;              // number of entries in FAT
        int mFatStartAddress;            // start address of FAT
        int mDataStartAddress;           // start address of data blocks

        unsigned int SIZE = sum_sizeof(mSignature, mDiskSize, mClusterSize, mClusterCount,
                                       mFatEntryCount, mFatStartAddress, mDataStartAddress);
        BootSector() = default;
        ~BootSector() = default;
};

class FAT {
    public:
        enum class FLAG {
            UNUSED, FILE_END, BAD_CLUSTER
        };


};

class DirectoryItem {
    public:
        std::array<char, 12> filename;  // 7 + 1 + 3 + \0
        bool mIsFile;                    // true -> is file, false -> is dir
        int mSize;                       // file size
        int mStartCluster;               // first cluster of file

        unsigned int SIZE = sum_sizeof(filename, mIsFile, mSize, mStartCluster);
        DirectoryItem() = default;
        ~DirectoryItem() = default;
};

/**
 *
 */
class FAT_Filesystem : public IFilesystem {
    private:
        std::fstream mFS;

        // filesystem areas
        BootSector mBS;
        FAT mFAT;
            // data area
//        RootDirectory mRD;
//        ClusterArea mCA;

    public:
        FAT_Filesystem(const std::string& name, unsigned int diskSize);
        ~FAT_Filesystem() = default;

        virtual bool fs_creat(const std::vector<std::string>& args) override;
        virtual bool fs_open(const std::vector<std::string>& args) override;
        virtual bool fs_read(const std::vector<std::string>& args) override;
        virtual bool fs_write(const std::vector<std::string>& args) override;
    //        virtual bool fs_lseek(const std::vector<std::string>& args) = 0;
        virtual bool fs_close(const std::vector<std::string>& args) override;

};
