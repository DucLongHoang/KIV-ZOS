#pragma once

#include <array>
#include <fstream>
#include <utility>
#include <iostream>
#include <filesystem>

#include "IFilesystem.hpp"
#include "Initializable.hpp"
#include "utils.hpp"

using namespace std::string_literals;

/**
 * Class BootSector
 */
class BootSector : public InitializableFromDisk {
    public:
        std::string mSignature;             // author login
        unsigned int mDiskSize;             // total size of FS
        unsigned int mClusterSize;          // size of one cluster
        unsigned int mClusterCount;         // total number of clusters
        unsigned int mFatEntryCount;        // number of entries in FAT
        unsigned int mFatStartAddress;      // start address of FAT
        unsigned int mDataStartAddress;     // start address of data blocks

        unsigned int SIZE = sum_sizeof(mSignature, mDiskSize, mClusterSize, mClusterCount,
                                       mFatEntryCount, mFatStartAddress, mDataStartAddress);
        BootSector() = default;
        ~BootSector() = default;

        void init(unsigned int diskSize);
        void init_from_disk(std::fstream& stream, unsigned int pos) override;
};

/**
 * Class FAT - represents a File Allocation Table
 */
class FAT : public InitializableFromDisk {
    public:
        static constexpr int FLAG_UNUSED = -1;
        static constexpr int FLAG_FILE_END = -2;
        static constexpr int FLAG_BAD_CLUSTER = -3;
        static constexpr int FLAG_NO_FREE_SPACE = -4;

        std::vector<int> table;
        FAT() = default;
        ~FAT() = default;

        unsigned int SIZE;

        void init(unsigned int fatEntryCount);
        void init_from_disk(std::fstream& stream, unsigned int pos) override;

        void write_FAT(unsigned int startAddress, int idxOrFlag);
        unsigned int find_free_index();
};

/**
 * Class DirectoryItem - can be file or directory
 */
class DirectoryItem : public InitializableFromDisk {
    public:
        std::string mFilename;          // 7 + 1 + 3 + \0
        bool mIsFile;                   // true -> is file, false -> is dir
        unsigned int mSize;             // file size
        unsigned int mStartCluster;     // first cluster of file
        unsigned int SIZE = sum_sizeof(mFilename, mIsFile, mSize, mStartCluster);

        DirectoryItem() = default;
        ~DirectoryItem() = default;
        void init(const std::string& filename, bool isFile, int size, int startCLuster);
        void init_from_disk(std::fstream& stream, unsigned int pos) override;
};

/**
 * Class FAT_Filesystem
 */
class FAT_Filesystem : public IFilesystem {
    private:
        std::fstream mFileStream;
        std::string mDiskName;

        BootSector mBS;
        FAT mFAT;
        DirectoryItem mRootDir;

    public:
        FAT_Filesystem(std::string name) : mDiskName(std::move(name)) {}
        ~FAT_Filesystem() = default;

        bool init_fs(const std::vector<std::any> &args) override;
        bool mount_fs(const std::vector<std::any> &args) override;

        unsigned int fs_creat(const std::vector<std::any>& args) override;
        unsigned int fs_open(const std::vector<std::any>& args) override;
        unsigned int fs_read(const std::vector<std::any>& args) override;
        unsigned int fs_write(const std::vector<std::any>& args) override;
//        unsigned int fs_lseek(const std::vector<std::any>& args) = 0;
        bool fs_close(const std::vector<std::any>& args) override;
};
