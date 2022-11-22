#pragma once

#include <array>
#include <string>
#include <vector>
#include <fstream>
#include <utility>
#include <iostream>
#include <filesystem>

#include "utils.hpp"

/**
 *
 */
class IFilesystem {
    public:
        virtual bool fs_creat(const std::vector<std::string>& args) = 0;
        virtual bool fs_open(const std::vector<std::string>& args)  = 0;
        virtual bool fs_read(const std::vector<std::string>& args)  = 0;
        virtual bool fs_write(const std::vector<std::string>& args) = 0;
        virtual bool fs_lseek(const std::vector<std::string>& args) { return false; }
        virtual bool fs_close(const std::vector<std::string>& args) = 0;
};

/**
 *
 */
class FAT : public IFilesystem {

	private:
        std::fstream mFS;

        std::array<char,9> mSignature;   // author login
        int mDiskSize;                   // total size of FS
        int mClusterSize;                // size of one cluster
        int mClusterCount;               // total number of clusters
        int mFatEntryCount;              // number of entries in FAT
        int mFatStartAddress;            // start address of FAT
        int mDataStartAddress;           // start address of data blocks


	public:
        enum class FLAG {
            UNUSED, FILE_END, BAD_CLUSTER
        };

        class File {
            std::array<char, 12> filename;  // 7 + 1 + 3 + \0
            bool mIsFile;                    // true -> is file, false -> is dir
            int mSize;                       // file size
            int mStartCluster;               // first cluster of file

            unsigned int SIZE = sum_sizeof(filename, mIsFile, mSize, mStartCluster);
        };

        FAT(const std::string& name);
        ~FAT() = default;

        virtual bool fs_creat(const std::vector<std::string>& args) override;
        virtual bool fs_open(const std::vector<std::string>& args) override;
        virtual bool fs_read(const std::vector<std::string>& args) override;
        virtual bool fs_write(const std::vector<std::string>& args) override;
//        virtual bool fs_lseek(const std::vector<std::string>& args) = 0;
        virtual bool fs_close(const std::vector<std::string>& args) override;

        unsigned int SIZE() {
            return sum_sizeof(mSignature, mDiskSize, mClusterSize, mClusterCount, mFatEntryCount, mFatStartAddress,
                       mDataStartAddress);
        }
};
