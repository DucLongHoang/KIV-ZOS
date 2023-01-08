#pragma once

#include <fstream>
#include <optional>
#include "Utils.hpp"

/**
 * Class BootSector - contains the basic info about the filesystem.
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

        [[nodiscard]] uint SIZE() const {
            return SIGNATURE_LEN + Utils::sum_sizeof(mDiskSize, mClusterSize, mClusterCount,
                                                     mFatStartAddress, mDataStartAddress, mMaxDirEntries);
        }

        BootSector() = default;
        ~BootSector() = default;

        /**
         * The de-facto constructor.
         * @param diskSize to be initialized to
         */
        void init(uint diskSize);

        /**
         * Loads BootSector from a file.
         * @param stream
         */
        void mount(std::fstream& stream);

        /**
         * Method saves BootSector data to file.
         * @param stream
         */
        void write_to_disk(std::fstream& stream);
};

/**
 * Class FAT - represents a File Allocation Table.
 */
class FAT {
    public:
        static constexpr int FLAG_UNUSED = -1;
        static constexpr int FLAG_FILE_END = -2;
        static constexpr int FLAG_BAD_CLUSTER = -3;
        static constexpr int FLAG_NO_FREE_SPACE = -4;

        // FAT table
        std::vector<int> table;
        [[nodiscard]] uint SIZE() const { return table.size() * sizeof(uint); }

        FAT() = default;
        ~FAT() = default;

        /**
         * The de-facto constructor.
         * @param fatEntryCount
         */
        void init(uint fatEntryCount);

        /**
         * Loads FAT from a file.
         * @param stream
         */
        void mount(std::fstream& stream);

        /**
         * Writes FAT entries to file.
         * @param stream
         */
        void write_to_disk(std::fstream& stream);

        /**
         * Method modifies the FAT table.
         * @param idx starting index
         * @param fileSize of to-be-created file
         * @return true on success, else false (probably no more free space in FAT)
         */
        bool write_FAT(uint idx, size_t fileSize);

        /**
         * Method frees the FAT table starting from index idx.
         * @param idx index to start freeing from
         */
        void free_FAT(uint idx);

        /**
         * Method finds a free index in the FAT table.
         * @param ignoredIdx index to be ignored even if it's free.
         * @return free index or some FLAG
         */
        [[nodiscard]] int find_free_index(int ignoredIdx) const;

        // allows traversing the underlying std::vector<int> table (without exposing it)
        [[nodiscard]] std::vector<int>::const_iterator begin() const { return table.begin(); }
        [[nodiscard]] std::vector<int>::const_iterator end() const { return table.end(); }
};

/**
 * Class DirEntry - abstraction of a file ina FAT filesystem.
 */
class DirEntry {
    public:
        std::string mFilename;  // 7 + 1 + 3 + \0
        bool mIsFile;           // true -> is file, false -> is dir
        uint mSize;             // file size
        uint mStartCluster;     // first cluster of file

        [[nodiscard]] uint SIZE() const {
            return FILENAME_LEN + Utils::sum_sizeof(mIsFile, mSize, mStartCluster);
        }

        DirEntry() = default;
        ~DirEntry() = default;

        /**
         * Overloaded operator bool().
         * @return true if dirEntry is valid, otherwise false
         */
        explicit operator bool() const;

        /**
         * The de-facto constructor.
         * @param filename name of DirEntry
         * @param isFile type of DirEntry
         * @param size size of DirEntry
         * @param startCLuster starting cluster of DirEntry
         */
        void init(const std::string& filename, bool isFile, uint size, uint startCLuster);

        /**
         * Load DirEntry from file.
         * @param stream to be loaded from
         */
        void mount(std::fstream& stream);

        /**
         * Method saves DirEntry info to file.
         * @param stream to be written into
         */
        void write_to_disk(std::fstream& stream);

        /**
         * Method save contents of a DirEntry into a file.
         * @param stream to be written into.
         * @param dataStartAddress of DirEntry
         * @param clusters of DirEntry
         * @param content of DirEntry
         */
        void write_content_to_disk(std::fstream& stream, uint dataStartAddress, const std::vector<uint>& clusters , const std::string& content) const;
    };

/**
 * Class Filesystem - simplified FAT filesystem
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

        /**
         * The de-facto constructor.
         * @param size
         */
        void init(uint size);

        /**
         * Loads FS from a file.
         */
        void mount();

        /**
         * Method wipes all cluster, making the disk clean.
         */
        void wipe_all_clusters();

        /**
         * Initializes some default files for testing.
         */
        void init_default_files();

        /**
         * Method retrieves the first DirEntry from a cluster.
         * @param cluster to get the DirEntry from
         * @param isFile retrieved DirEntry file type
         * @param last if true, retrieves the last file instead
         * @return DirEntry at first or last position in a cluster
         */
        DirEntry get_dir_entry(uint cluster, bool isFile, bool last);

        /**
         * Method returns the position of a filename in a parent DirEntry.
         * @param searched file
         * @param parent DirEntry
         * @return position of searched file. Indexed from 0
         */
        int get_position(const std::string& searched, const DirEntry& parent);

        /**
         * Method returns the number of child DirEntries of a DirEntry. A directory is expected though.
         * @param dirEntry to get the child count of.
         * @return number of child dirEntries
         */
        uint get_child_dir_entry_count(const DirEntry& dirEntry);

        /**
         * Method creates a DiEntry from the given parameters. Checks for FAT fullness or name duplicate are done as well.
         * @param parentCluster to know where to save the new DirEntry
         * @param name of the new DirEntry
         * @param isFile file type of new DirEntry
         * @param content of new DirEntry
         * @return newly created DirEntry or std::nullopt
         */
        std::optional<DirEntry> create_dir_entry(uint parentCluster, const std::string& name, bool isFile, const std::string& content = "");

        /**
         * Method copies a DirEntry with a changed name.
         * @param parentCluster to know where to save the new DirEntry
         * @param toCopy DirEntry to be copied
         * @param nameOfCopy name of the new file
         * @return newly created copy of a DirEntry or std::nulopt
         */
        std::optional<DirEntry> copy_dir_entry(uint parentCluster, const DirEntry& toCopy, const std::string& nameOfCopy);

        /**
         * Method removes a DirEntry.
         * @param parentCluster of DirEntry to change his info
         * @param position of to-be-removed DirEntry in parent
         */
        void remove_dir_entry(uint parentCluster, uint position);

        /**
         * Method reads a DirEntry as a directory.
         * @param dirEntry to be read
         * @return vector of DirEntries - child of a directory
         */
        std::vector<DirEntry> read_dir_entry_as_dir(const DirEntry& dirEntry);

        /**
         * Method reads a DirEntry as a file.
         * @param dirEntry to be read
         * @return the contents of a file
         */
        std::string read_dir_entry_as_file(const DirEntry& dirEntry);

        /**
         * Method return all cluster locations of a DirEntry.
         * @param dirEntry whose locations we want to know
         * @return vector of locations
         */
        std::vector<uint> get_cluster_locations(const DirEntry& dirEntry);
};