#pragma once

#include <any>
#include <string>
#include <vector>

/**
 *
 */
class IFilesystem {
    public:
        // methods handling filesystem itself
        virtual bool init_fs(const std::vector<std::any>& args) = 0;
        virtual bool mount_fs(const std::vector<std::any>& args) = 0;
        // methods handling contents of filesystem
        virtual bool fs_creat(const std::vector<std::any>& args) = 0;
        virtual bool fs_open(const std::vector<std::any>& args)  = 0;
        virtual bool fs_read(const std::vector<std::any>& args)  = 0;
        virtual bool fs_write(const std::vector<std::any>& args) = 0;
        virtual bool fs_lseek(const std::vector<std::any>& args) { return false; }
        virtual bool fs_close(const std::vector<std::any>& args) = 0;
};