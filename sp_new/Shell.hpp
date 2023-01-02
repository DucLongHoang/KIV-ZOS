#pragma once

#include <memory>
#include <vector>
#include <functional>
#include <unordered_map>
#include "Utils.hpp"
#include "Filesystem.hpp"

using Arguments = std::vector<std::string>;
using Handler = std::function<bool (Arguments&)>;

class Shell {
    private:
        std::string mFsName;
        std::string mCWD;
        uint mCWC;    // current working cluster
        std::unique_ptr<Filesystem> mFilesystem;
        std::unordered_map<std::string, Handler> mHandlerMap;
        std::unordered_map<std::string, Range> mArgsCountMap;

        void fill_handlers();
        void fill_args_count();
        bool check_args_count(const std::string& opcode, uint argc);
        void mount(const std::string& fsName);

    public:
        explicit Shell(const std::string& fsName);
        ~Shell() = default;
        void run(std::istream& istream);
};