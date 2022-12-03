#pragma once

#include <string>

class InitializableFromDisk {
    public:
        virtual void init_from_disk(std::fstream& stream, unsigned int pos) = 0;
};