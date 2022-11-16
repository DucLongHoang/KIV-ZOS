#pragma once

#include <array>
#include <string>
#include <vector>
#include <fstream>
#include <utility>
#include <iostream>

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
        std::fstream fs;

        std::array<char,9> signature;   // author login
        int diskSize;                   // total size of FS
        int clusterSize;                // size of one cluster
        int clusterCount;               // total number of clusters
        int fatEntryCount;              // number of entries in FAT
        int fatStartAddress;            // start address of FAT
        int dataStartAddress;           // start address of data blocks


	public:
        enum class FLAG {
            UNUSED, FILE_END, BAD_CLUSTER
        };

        class File {
            std::array<char, 12> filename;  // 7 + 1 + 3 + \0
            bool isFile;                    // true -> is file, false -> is dir
            int size;                       // file size
            int startCluster;               // first cluster of file
        };

        FAT(const std::string& name) {
            fs.open(name, std::ios::out | std::ios::in | std::ios::app | std::ios::binary);
            if (!fs.is_open())
                std::cout << "Could not mount filesystem: " << name << std::endl;
        }
        ~FAT() = default;

        virtual bool fs_creat(const std::vector<std::string>& args) override;
        virtual bool fs_open(const std::vector<std::string>& args) override;
        virtual bool fs_read(const std::vector<std::string>& args) override;
        virtual bool fs_write(const std::vector<std::string>& args) override;
//        virtual bool fs_lseek(const std::vector<std::string>& args) = 0;
        virtual bool fs_close(const std::vector<std::string>& args) override;
};
