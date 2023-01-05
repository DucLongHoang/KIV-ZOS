#pragma once

#include <memory>
#include <vector>
#include <optional>
#include <functional>
#include <unordered_map>
#include "Utils.hpp"
#include "Filesystem.hpp"

using Arguments = std::vector<std::string>;
using Handler = std::function<bool (Arguments&)>;

class Shell {
    private:
        enum class DirEntryType { DIR, FILE, BOTH };

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
        std::optional<DirEntry> get_dir_entry_from_path(const std::string& path, DirEntryType type);

    public:
        explicit Shell(const std::string& fsName);
        ~Shell() = default;
        void run(std::istream& istream);
};