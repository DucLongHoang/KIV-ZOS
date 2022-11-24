#pragma once

#include <string>
#include <vector>

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