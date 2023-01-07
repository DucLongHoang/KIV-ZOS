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
        uint mFatStartAddress;      // start address of FAT
        uint mDataStartAddress;     // start address of data blocks
        uint mMaxDirEntries;        // max number of dirEntries in a dir cluster

        uint SIZE() const {
            return SIGNATURE_LEN + Utils::sum_sizeof(mDiskSize, mClusterSize, mClusterCount,
                                                     mFatStartAddress, mDataStartAddress, mMaxDirEntries);
        }

        BootSector() = default;
        ~BootSector() = default;

        void init(uint diskSize);
        void mount(std::fstream& stream, uint pos);
        void write_to_disk(std::fstream& stream);
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

        // FAT table
        std::vector<int> table;
        uint SIZE() const { return table.size() * sizeof(uint); }

        FAT() = default;
        ~FAT() = default;

        void init(uint fatEntryCount);
        void mount(std::fstream& stream, uint pos);
        void write_to_disk(std::fstream& stream);

        void write_FAT(uint idx, int fileSize);
        void free_FAT(uint idx);
        int find_free_index(int ignoredIdx) const;

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

        uint SIZE() const {
            return FILENAME_LEN + Utils::sum_sizeof(mIsFile, mSize, mStartCluster);
        }

        DirEntry() = default;
        ~DirEntry() = default;

        explicit operator bool() const;

        void init(const std::string& filename, bool isFile, uint size, uint startCLuster);
        void mount(std::fstream& stream);
        void write_to_disk(std::fstream& stream);
        void write_content_to_disk(std::fstream& stream, uint dataStartAddress, const std::vector<uint>& clusters , const std::string& content) const;
    };

/**
 * Class Filesystem
 */
class Filesystem {
    private:
        std::fstream mFileStream;
        std::string mDiskName;
        uint mTwoDirEntries;
        std::array<char, CLUSTER_SIZE> mEmptyCluster;

        BootSector mBS;
        FAT mFAT;
        DirEntry mRootDir;

    public:
        explicit Filesystem(std::string name) : mDiskName(std::move(name)), mTwoDirEntries(2)  {
            mEmptyCluster.fill('\0');
        }
        ~Filesystem() = default;

        void init(uint size);
        void mount();

        void wipe_all_clusters();
        void init_default_files();
        DirEntry get_root_dir();
        DirEntry get_dir_entry(uint cluster, bool isFile, bool last);
        int get_position(const std::string& searched, const DirEntry& parent);
        uint get_child_dir_entry_count(const DirEntry& dirEntry);
        std::optional<DirEntry> create_dir_entry(uint parentCluster, const std::string& name, bool isFile, const std::string& content = "");
        std::optional<DirEntry> copy_dir_entry(uint parentCluster, const DirEntry& toCopy, const std::string& nameOfCopy);
        void remove_dir_entry(uint parentCluster, uint position);
        std::vector<DirEntry> read_dir_entry_as_dir(const DirEntry& dirEntry);
        std::string read_dir_entry_as_file(const DirEntry& dirEntry);
        std::vector<uint> get_cluster_locations(const DirEntry& dirEntry);
};