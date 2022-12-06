#pragma once

#include <any>
#include <string>
#include <vector>

using uint = unsigned int;

/**
 *
 */
class IFilesystem {
    public:
        // methods handling filesystem itself
        virtual bool init_fs(const std::vector<std::any>& args) = 0;
        virtual bool mount_fs(const std::vector<std::any>& args) = 0;
        // methods handling contents of filesystem
        virtual uint fs_creat(const std::vector<std::any>& args) = 0;
        virtual uint fs_open(const std::vector<std::any>& args)  = 0;
        virtual uint fs_read(std::vector<std::any>& args)  = 0;
        virtual uint fs_write(const std::vector<std::any>& args) = 0;
        virtual bool fs_lseek(const std::vector<std::any>& args) { return false; }
        virtual bool fs_close(const std::vector<std::any>& args) = 0;
};