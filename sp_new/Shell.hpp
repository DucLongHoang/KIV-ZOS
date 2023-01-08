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

/**
 * Class Shell - a layer above the Filesystem that manipulates it using linux commands.
 */
class Shell {
    private:
        /**
         * Enum class DirEntryType - simple enumeration, used in Shell::get_dir_entry_from_path().
         */
        enum class DirEntryType { DIR, FILE, BOTH };

        std::string mFsName;
        std::string mCWD;
        uint mCWC;    // current working cluster
        std::unique_ptr<Filesystem> mFilesystem;
        std::unordered_map<std::string, Handler> mHandlerMap;
        std::unordered_map<std::string, Range> mArgsCountMap;

        /**
         * Method fills out the handler map with Handlers - functions.
         */
        void fill_handlers();

        /**
         * Method fills a map for argument count checking.
         */
        void fill_args_count();

        /**
         * Method checks if correct argument count was received.
         * @param opcode type of command
         * @param argc argument count received
         * @return true if count is okay, else false
         */
        bool check_args_count(const std::string& opcode, uint argc);

        /**
         * Mounts the FS.
         * @param fsName name of FS to be mounted
         */
        void mount(const std::string& fsName);

        /**
         * Method parses the path to get the correct DirEntry. This method is the most important.
         * @param path to traverse to find file
         * @param type pf file we want (file, dir, doesn't matter)
         * @return DirEntry corresponding to path or std::nullopt
         */
        std::optional<DirEntry> get_dir_entry_from_path(const std::string& path, DirEntryType type);

    public:
        explicit Shell(const std::string& fsName);
        ~Shell() = default;

        /**
         * Method launches the Shell.
         * @param istream source of incoming commands
         */
        void run(std::istream& istream);
};