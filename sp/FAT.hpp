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
        uint mDiskSize;             // total size of FS
        uint mClusterSize;          // size of one cluster
        uint mClusterCount;         // total number of clusters
        uint mFatEntryCount;        // number of entries in FAT
        uint mFatStartAddress;      // start address of FAT
        uint mDataStartAddress;     // start address of data blocks

        uint SIZE = sum_sizeof(mSignature, mDiskSize, mClusterSize, mClusterCount,
                                       mFatEntryCount, mFatStartAddress, mDataStartAddress);
        BootSector() = default;
        ~BootSector() = default;

        void init(uint diskSize);
        void init_from_disk(std::fstream& stream, uint pos) override;
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

        uint SIZE;

        void init(uint fatEntryCount);
        void init_from_disk(std::fstream& stream, uint pos) override;

        void write_FAT(uint startAddress, int idxOrFlag);
        uint find_free_index();
};

/**
 * Class DirectoryItem - can be file or directory
 */
class DirectoryItem : public InitializableFromDisk {
    public:
        std::string mFilename;          // 7 + 1 + 3 + \0
        bool mIsFile;                   // true -> is file, false -> is dir
        uint mSize;             // file size
        uint mStartCluster;     // first cluster of file
        uint SIZE = sum_sizeof(mFilename, mIsFile, mSize, mStartCluster);

        DirectoryItem() = default;
        ~DirectoryItem() = default;
        void init(const std::string& filename, bool isFile, uint size, uint startCLuster);
        void init_from_disk(std::fstream& stream, uint pos) override;
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

        uint fs_creat(const std::vector<std::any>& args) override;
        uint fs_open(const std::vector<std::any>& args) override;
        uint fs_read(const std::vector<std::any>& args) override;
        uint fs_write(const std::vector<std::any>& args) override;
//        uint fs_lseek(const std::vector<std::any>& args) = 0;
        bool fs_close(const std::vector<std::any>& args) override;
};
