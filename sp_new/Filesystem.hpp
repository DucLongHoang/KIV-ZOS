#pragma once

#include <fstream>
#include "Utils.hpp"

/**
 * Class BootSector
 */
class BootSector {
    public:
        std::string mSignature;     // author login
        uint mDiskSize;             // total size of FS
        uint mClusterSize;          // size of one cluster
        uint mClusterCount;         // total number of clusters
        uint mFatEntryCount;        // number of entries in FAT
        uint mFatStartAddress;      // start address of FAT
        uint mDataStartAddress;     // start address of data blocks

        [[nodiscard]] uint size() const {
            return mSignature.size() + Utils::sum_sizeof(mDiskSize, mClusterSize, mClusterCount,
                                             mFatEntryCount, mFatStartAddress, mDataStartAddress);
        }

        BootSector() = default;
        ~BootSector() = default;

        void init(uint diskSize);
        void init_from_disk(std::fstream& stream, uint pos);
};

/**
 * Class FAT - represents a File Allocation Table
 */
class FAT {
    public:
        static constexpr int FLAG_UNUSED = -1;
        static constexpr int FLAG_FILE_END = -2;
        static constexpr int FLAG_BAD_CLUSTER = -3;
        static constexpr int FLAG_NO_FREE_SPACE = -4;

        std::vector<int> table;

        [[nodiscard]] uint size() const { return table.size() * sizeof(uint); }

        FAT() = default;
        ~FAT() = default;

        void init(uint fatEntryCount);
        void init_from_disk(std::fstream& stream, uint pos);

        void write_FAT(uint startAddress, int idxOrFlag);
        [[nodiscard]] uint find_free_index() const;

        [[nodiscard]] std::vector<int>::const_iterator begin() const { return table.begin(); }
        [[nodiscard]] std::vector<int>::const_iterator end() const { return table.end(); }
};

/**
 * Class DirEntry - can be file or directory
 */
class DirEntry {
public:
    std::string mFilename;  // 7 + 1 + 3 + \0
    bool mIsFile;           // true -> is file, false -> is dir
    uint mSize;             // file size
    uint mStartCluster;     // first cluster of file

    [[nodiscard]] uint size() const {
        return mFilename.size() + Utils::sum_sizeof(mIsFile, mSize, mStartCluster);
    }

    DirEntry() = default;
    ~DirEntry() = default;

    void init(const std::string& filename, bool isFile, uint size, uint startCLuster);
    void init_from_disk(std::fstream& stream, uint pos);
};

/**
 * Class Filesystem
 */
class Filesystem {
private:
    std::fstream mFileStream;
    std::string mDiskName;

    std::unique_ptr<BootSector> mBS;
    std::unique_ptr<FAT> mFAT;
    std::unique_ptr<DirEntry> mRootDir;

public:
    Filesystem(std::string name) : mDiskName(std::move(name)) {}
    ~Filesystem() {
        mFileStream.close();
    };

    void wipe_clusters();
    void init_test_files();

    void write_boot_sector();
    void write_FAT();
    void write_root_dir();
    void write_directory_item(DirEntry& dirItem);

    bool init_fs(uint size) override;
    bool mount_fs() override;
};