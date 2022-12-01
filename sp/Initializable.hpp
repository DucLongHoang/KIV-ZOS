#pragma once

#include <string>

class InitializableFromDisk {
    public:
        virtual bool init_from_disk(const std::string &str) = 0;
};