#pragma once

#include <fstream>
#include "Utils.hpp"

//using Cluster = std::array<char, CLUSTER_SIZE>;
//using Clusters = std::vector<Cluster>;

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

        uint size() const {
            return mSignature.size() + Utils::sum_sizeof(mDiskSize, mClusterSize, mClusterCount,
                                                         mFatStartAddress, mDataStartAddress);
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
        uint size() const { return table.size() * sizeof(uint); }

        FAT() = default;
        ~FAT() = default;

        void init(uint fatEntryCount);
        void mount(std::fstream& stream, uint pos);
        void write_to_disk(std::fstream& stream);

        void write_FAT(uint idx, uint fileSize);
        uint find_free_index() const;
        uint find_index_of(const std::string& str, uint startingPoint) const;

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

        uint size() const {
            return mFilename.size() + Utils::sum_sizeof(mIsFile, mSize, mStartCluster);
        }

        DirEntry() = default;
        ~DirEntry() = default;

        explicit operator bool() const;

        void init(const std::string& filename, bool isFile, uint size, uint startCLuster);
        void mount(std::fstream& stream, uint pos);
        void write_to_disk(std::fstream& stream);
        void write_content_to_disk(std::fstream& stream, uint dataStartAddress, const std::vector<uint>& clusters , const std::string& content);
    };

/**
 * Class Filesystem
 */
class Filesystem {
    private:
        std::fstream mFileStream;
        std::string mDiskName;

        BootSector mBS;
        FAT mFAT;
        DirEntry mRootDir;
//        Clusters clusters;

    public:
        explicit Filesystem(std::string name) : mDiskName(std::move(name)) {}
        ~Filesystem() = default;

        void init(uint size);
        void mount();

        void wipe_clusters();
        void init_default_files();
        DirEntry get_dir_entry(uint cluster);
        void create_dir_entry(uint parentCluster, const std::string& name, bool isFile, const std::string& content);
        void remove_dir_entry(const std::string& name);
        std::vector<DirEntry> read_dir_entry_as_dir(const DirEntry& parentDir);
        std::string read_dir_entry_as_file(const DirEntry& dirEntry);
        std::vector<uint> get_cluster_locations(const std::string& dirEntryName);
};