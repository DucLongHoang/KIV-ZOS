#include "Shell.hpp"

#include <regex>
#include <sstream>
#include <filesystem>

using namespace std::string_literals;

static const std::regex REGEX_FORMAT("[1-9]+[0-9]*(kb|Kb|KB|mb|Mb|MB)");
static const std::regex REGEX_KB("(kb|Kb|KB){1}");

Shell::Shell(const std::string& fsName) : mFsName(fsName), mCWD("/"), mCWC(0) {
    fill_args_count();
    fill_handlers();

    if (std::filesystem::exists(fsName)) {
        std::cout << "File system: " << fsName << " found" << std::endl;
        std::cout << "Mounting file system..." << std::endl;
        mount(fsName);
    }
    else {
        std::cout << "File system: " << fsName << " not found" << std::endl;
        std::cout << "Create new disk by using cmd: 'format [x][y]'" << std::endl;
        std::cout << "[x] = positive integer" << std::endl;
        std::cout << "[y] = KB or MB (case sensitive)" << std::endl;
    }
}

void Shell::fill_handlers() {
    mHandlerMap["cp"] = [this](Arguments& args) -> bool {
        std::filesystem::path fromPath(args.front());
        std::filesystem::path toPath(args.back());

        // check if source file exists
        std::optional<DirEntry> fileToCopy = Shell::get_dir_entry_from_path(fromPath.string(), DirEntryType::BOTH);
        if (!fileToCopy) return true;
        if (!fileToCopy->mIsFile) {
            std::cout << Utils::remove_padding(fileToCopy->mFilename) << " is a directory" << std::endl;
            return true;
        }

        // check if target location exists
        std::optional<DirEntry> targetDir = Shell::get_dir_entry_from_path(toPath.parent_path().string(), DirEntryType::DIR);
        if (!targetDir) return true;

        // copy source file - no need to check, if it exists it just won't be created
        mFilesystem->copy_dir_entry(targetDir->mStartCluster, fileToCopy.value(), toPath.filename().string());

        return true;
    };
    mHandlerMap["mv"] = [this](Arguments& args) -> bool {
        std::filesystem::path fromPath(args.front());
        std::filesystem::path toPath(args.back());

        // check if source file exists
        std::optional<DirEntry> fileToMove = Shell::get_dir_entry_from_path(fromPath.string(), DirEntryType::BOTH);
        if (!fileToMove) return true;
        if (!fileToMove->mIsFile) {
            std::cout << Utils::remove_padding(fileToMove->mFilename) << " is a directory" << std::endl;
            return true;
        }

        // check if target location exists
        std::optional<DirEntry> targetDir = Shell::get_dir_entry_from_path(toPath.parent_path().string(), DirEntryType::DIR);
        if (!targetDir) return true;

        // getting parent cluster of to-be-moved file
        std::optional<DirEntry> sourceDir = Shell::get_dir_entry_from_path(fromPath.parent_path().string(), DirEntryType::DIR);

        // copy source file
        auto opt = mFilesystem->copy_dir_entry(targetDir->mStartCluster, fileToMove.value(), toPath.filename().string());

        // check if it is a duplicate - end prematurely to not delete source file
        if (!opt) return true;

        // remove source file
        auto position = mFilesystem->get_position(Utils::remove_padding(fileToMove->mFilename), sourceDir.value());
        mFilesystem->remove_dir_entry(sourceDir->mStartCluster, position);

        return true;
    };
    mHandlerMap["rm"] = [this](Arguments& args) -> bool {
        std::filesystem::path path(args.front());

        // sanity check
        if (path.filename().string() == "." || path.filename().string() == "..") {
            std::cout << "Cannot remove " << path.filename().string() << std::endl;
            std::cout << "Try: 'rmdir " << path.filename().string() << "'" << std::endl;
            return true;
        }

        // find dir of to-be-removed file
        auto dir = Shell::get_dir_entry_from_path(path.parent_path().string(), DirEntryType::DIR);
        if (!dir) return true;

        auto position = mFilesystem->get_position(path.filename().string(), dir.value());
        if (position >= 0)
            mFilesystem->remove_dir_entry(dir->mStartCluster, position);
        else
            std::cout << path.filename().string() << " - no such file" << std::endl;

        return true;
    };
    mHandlerMap["mkdir"] = [this](Arguments& args) -> bool {
        std::filesystem::path path(args.front());
        std::string dirName = path.filename().string();

        if (dirName.size() > FILENAME_LEN) {
            std::cout << "Directory name: " << dirName << " is too long" << std::endl;
            return true;
        }

        std::optional<DirEntry> parentDir = Shell::get_dir_entry_from_path(path.parent_path().string(), DirEntryType::DIR);
        if (!parentDir) return true;

        // no extra precautions
        mFilesystem->create_dir_entry(parentDir->mStartCluster, dirName, false);
        return true;
    };
    mHandlerMap["rmdir"] = [this](Arguments& args) -> bool {
        std::filesystem::path path(args.front());

        // sanity check
        if (path.filename().string() == "." || path.filename().string() == "..") {
            std::cout << "LOL, you really tried 'rmdir " << path.filename().string() << "'" << std::endl;
            return true;
        }

        // find dir of to-be-removed dir
        auto dir = Shell::get_dir_entry_from_path(path.parent_path().string(), DirEntryType::DIR);
        if (!dir) return true;

        auto position = mFilesystem->get_position(path.filename().string(), dir.value());
        if (position >= 0)
            mFilesystem->remove_dir_entry(dir->mStartCluster, position);
        else
            std::cout << path.filename().string() << " - no such directory" << std::endl;

        return true;
    };
    mHandlerMap["ls"] = [this](Arguments& args) -> bool {
        DirEntry dir;
        if (args.empty())
            dir = mFilesystem->get_dir_entry(mCWC, false, false);
        else {
            std::optional<DirEntry> dirEntryToList = Shell::get_dir_entry_from_path(args.front(), DirEntryType::DIR);
            if (!dirEntryToList) return true;
            dir = dirEntryToList.value();
        }

        auto dirEntries = mFilesystem->read_dir_entry_as_dir(dir);
        for (auto& dirEntry : dirEntries) {
            std::cout << Utils::remove_padding(dirEntry.mFilename) << ((dirEntry.mIsFile) ? "" : "/") << std::endl;
        }
        return true;
    };
    mHandlerMap["cat"] = [this](Arguments& args) -> bool {
        std::optional<DirEntry> fileToCat = Shell::get_dir_entry_from_path(args.front(), DirEntryType::FILE);
        if (!fileToCat) return true;

        std::cout << mFilesystem->read_dir_entry_as_file(fileToCat.value()) << std::endl;
        return true;
    };
    mHandlerMap["cd"] = [this](Arguments& args) -> bool {
        if (args.front() == ".") return true;
        if (args.front() == "/") {
            mCWD = "/";
            mCWC = 0;
            return true;
        }

        DirEntry curDir = mFilesystem->get_dir_entry(mCWC, false, false);
        auto dirEntries = mFilesystem->read_dir_entry_as_dir(curDir);

        for (auto& dirEntry : dirEntries) {
            if (Utils::remove_padding(dirEntry.mFilename) == args.front()) {
                if (dirEntry.mIsFile) {
                    std::cout << args.front() << " is a file" << std::endl;
                    return true;
                }
                if (dirEntry.mStartCluster == 0)    // selected dirEntry is root "/"
                    mCWD = "/";
                else {
                    auto dirTo = Utils::remove_padding(dirEntry.mFilename);
                    dirTo = (mCWD == "/") ? dirTo : "/"s.append(dirTo);
                    mCWD = (args.front() == "..")
                           ? mCWD.substr(0, mCWD.find_last_of('/'))
                           : mCWD += dirTo;
                }
                mCWC = dirEntry.mStartCluster;
                return true;
            }
        }
        std::cout << args.front() << " - no such directory" << std::endl;
        return true;
    };
    mHandlerMap["pwd"] = [this](Arguments& args) -> bool {
        std::cout << mCWD << std::endl;
        return true;
    };
    mHandlerMap["info"] = [this](Arguments& args) -> bool {
        std::optional<DirEntry> fileToInfo = Shell::get_dir_entry_from_path(args.front(), DirEntryType::BOTH);
        if (!fileToInfo) return true;

        auto clusters = mFilesystem->get_cluster_locations(fileToInfo.value());

        // format output, cluster IDs seperated by comma
        std::ostringstream oss{"File: " + Utils::remove_padding(fileToInfo->mFilename) + " is in cluster/s: "};
        for (auto& i : clusters) {
            oss << i << ", ";
        }
        auto str = oss.str();
        str.pop_back();
        str.pop_back();

        std::cout << str << std::endl;
        return true;
    };
    mHandlerMap["incp"] = [this](Arguments& args) -> bool {
        std::filesystem::path fromPath(args.front());
        std::filesystem::path toPath(args.back());

        // check if source file exists
        std::ifstream ifs(fromPath, std::ios::binary);
        if (!ifs.is_open()) {
            std::cout << fromPath.filename().string() << " - no such file" << std::endl;
            return true;
        }

        // check if target location exists
        std::optional<DirEntry> targetDir = Shell::get_dir_entry_from_path(toPath.parent_path().string(), DirEntryType::DIR);
        if (!targetDir) return true;

        // save file into string
        std::stringstream ss;
//        while (ifs.good()) {
//            ss.put(ifs.get());
//        }
        ss << ifs.rdbuf();
        const auto fileContent = ss.str();

        // copy source file - no extra precautions
        mFilesystem->create_dir_entry(targetDir->mStartCluster, toPath.filename().string(), true, fileContent);
        return true;
    };
    mHandlerMap["outcp"] = [this](Arguments& args) -> bool {
        std::filesystem::path fromPath(args.front());
        std::filesystem::path toPath(args.back());

        // check if source file exists
        std::optional<DirEntry> fileToCopy = Shell::get_dir_entry_from_path(fromPath.string(), DirEntryType::BOTH);
        if (!fileToCopy) return true;
        if (!fileToCopy->mIsFile) {
            std::cout << Utils::remove_padding(fileToCopy->mFilename) << " is a directory" << std::endl;
            return true;
        }

        // check if target location exists
        std::ofstream ofs(toPath, std::ios::binary);
        if (!ofs.is_open()) {
            std::cout << toPath.parent_path().string() << " - no such file" << std::endl;
            return true;
        }

        // print file content into string
        const auto fileContent = mFilesystem->read_dir_entry_as_file(fileToCopy.value());
        ofs << fileContent;

        return true;
    };
    mHandlerMap["load"] = [this](Arguments& args) -> bool {
        std::filesystem::path fromPath(args.front());

        // check if source file exists
        std::ifstream ifs(fromPath);
        if (!ifs.is_open()) {
            std::cout << fromPath.filename().string() << " - no such file" << std::endl;
            return true;
        }

        // do the same thing as in the Shell::run() method
        while (not ifs.eof()) {
            args.clear();
            std::string command, opcode, arg;

            // prompt
            std::cout << std::endl << "root@root:" << mCWD << std::endl;
            std::cout << "$";
            std::getline(ifs, command);
            if (Utils::is_white_space(command)) continue;

            // print out input if automated
            std::cout << command << std::endl;

            // find space
            size_t pos = command.find_first_of(' ');
            if (pos != std::string::npos) {
                // if space, parse command and args
                opcode = command.substr(0, pos);

                std::stringstream stream{command.substr(pos + 1)};
                // load vector with parsed args
                while (std::getline(stream, arg, ' ')) {
                    args.emplace_back(arg);
                }
            }
                // the opcode is the whole inputted line
            else opcode = command;

            // check if command exists
            if (!mHandlerMap.contains(opcode)) {
                std::cout << "Invalid command: " << opcode << std::endl;
                continue;
            }
            if (!check_args_count(opcode, args.size())) {
                std::cout << "Incorrect number of arguments" << std::endl;
                continue;
            }
            mHandlerMap[opcode](args);
        }
        return true;
    };
    mHandlerMap["format"] = [this](Arguments& args) -> bool {
        if (!std::regex_match(args[0], REGEX_FORMAT)) {
            std::cout << "Invalid input: " << "format " << args[0] << std::endl;
            std::cout << "Try e.g.     : " << "format 200KB" << std::endl;
            return true;
        }

        std::string arg(args[0]);
        std::string_view msg = std::filesystem::exists(mFsName) ?
                          "Formatting existing disk..." : "Creating new disk...";
        std::cout << msg << std::endl;
        mFilesystem = std::make_unique<Filesystem>(mFsName);

        auto bytes = arg.substr(arg.length() - 2);
        auto multiplier = std::regex_match(bytes, REGEX_KB) ? 1_KB : 1_MB;
        auto diskSize = std::stoi(arg.substr(0, arg.size())) * multiplier;

        mFilesystem->init(diskSize);
        mCWD = "/";
        mCWC = 0;

        return true;
    };
    mHandlerMap["xcp"] = [this](Arguments& args) -> bool {
        std::filesystem::path fromPath1(args.front());
        std::filesystem::path fromPath2(args[1]);
        std::filesystem::path toPath(args.back());

        // check if first source file exists
        std::optional<DirEntry> fileToCopy1 = Shell::get_dir_entry_from_path(fromPath1.string(), DirEntryType::BOTH);
        if (!fileToCopy1) return true;
        if (!fileToCopy1->mIsFile) {
            std::cout << Utils::remove_padding(fileToCopy1->mFilename) << " is a directory" << std::endl;
            return true;
        }

        // check if second source file exists
        std::optional<DirEntry> fileToCopy2 = Shell::get_dir_entry_from_path(fromPath2.string(), DirEntryType::BOTH);
        if (!fileToCopy2) return true;
        if (!fileToCopy2->mIsFile) {
            std::cout << Utils::remove_padding(fileToCopy2->mFilename) << " is a directory" << std::endl;
            return true;
        }

        // check if target location exists
        std::optional<DirEntry> targetDir = Shell::get_dir_entry_from_path(toPath.parent_path().string(), DirEntryType::DIR);
        if (!targetDir) return true;

        auto combinedContent = mFilesystem->read_dir_entry_as_file(fileToCopy1.value())
                        .append(mFilesystem->read_dir_entry_as_file(fileToCopy2.value()));

        // create aggregate file - no extra precautions
        mFilesystem->create_dir_entry(targetDir->mStartCluster, toPath.filename().string(), true, combinedContent);
        return true;
    };
    mHandlerMap["short"] = [this](Arguments& args) -> bool {
        std::filesystem::path path(args.front());

        // check if source file exists
        std::optional<DirEntry> fileToShort = Shell::get_dir_entry_from_path(path.string(), DirEntryType::BOTH);
        auto name = Utils::remove_padding(fileToShort->mFilename);

        if (!fileToShort) return true;
        if (!fileToShort->mIsFile) {
            std::cout << name << " is a directory" << std::endl;
            return true;
        }
        if (fileToShort->mSize <= SHORT_THRESHOLD) {
            std::cout << name << " does not need to be shorted" << std::endl;
            return true;
        }

        // get parent dir of shorted file - no need to check
        std::optional<DirEntry> dir = Shell::get_dir_entry_from_path(path.parent_path().string(), DirEntryType::DIR);
        auto shortedContent = mFilesystem->read_dir_entry_as_file(fileToShort.value()).substr(0, SHORT_THRESHOLD);

        // remove original file
        auto pos = mFilesystem->get_position(name, dir.value());
        mFilesystem->remove_dir_entry(dir->mStartCluster, pos);

        // create original file with shorted content
        mFilesystem->create_dir_entry(dir->mStartCluster, name, true, shortedContent);

        return true;
    };
    mHandlerMap["exit"] = [](Arguments& args) -> bool {
        return false;
    };
    mHandlerMap["quit"] = [](Arguments& args) -> bool {
        return false;
    };
    mHandlerMap["close"] = [](Arguments& args) -> bool {
        return false;
    };
}

void Shell::fill_args_count() {
    mArgsCountMap["cp"] = Range{2, 2};
    mArgsCountMap["mv"] = Range{2, 2};
    mArgsCountMap["rm"] = Range{1, 1};
    mArgsCountMap["mkdir"] = Range{1, 1};
    mArgsCountMap["rmdir"] = Range{1, 1};
    mArgsCountMap["ls"] = Range{0, 1};
    mArgsCountMap["cat"] = Range{1, 1};
    mArgsCountMap["cd"] = Range{1, 1};
    mArgsCountMap["pwd"] = Range{0, 0};
    mArgsCountMap["info"] = Range{1, 1};
    mArgsCountMap["incp"] = Range{2, 2};
    mArgsCountMap["outcp"] = Range{2, 2};
    mArgsCountMap["load"] = Range{1, 1};
    mArgsCountMap["format"] = Range{1, 1};
    mArgsCountMap["xcp"] = Range{3, 3};
    mArgsCountMap["short"] = Range{1, 1};
    mArgsCountMap["exit"] = Range{0, 0};
    mArgsCountMap["quit"] = Range{0, 0};
    mArgsCountMap["close"] = Range{0, 0};
}

bool Shell::check_args_count(const std::string& opcode, uint argc) {
    auto range = mArgsCountMap[opcode];
    return (range.lower <= argc && argc <= range.upper);
}

std::optional<DirEntry> Shell::get_dir_entry_from_path(const std::string& path, DirEntryType type) {
    std::filesystem::path fullPath(path);
    std::vector<std::string> pathParts{};

    // init starting point from root or CWD
    uint startCluster = path.starts_with('/') ? 0 : mCWC;
    DirEntry curDirEntry = mFilesystem->get_dir_entry(startCluster, false, false);

    // parsing path into parts
    auto child = fullPath.filename().string();
    auto parent = fullPath.parent_path().string();
    while (!child.empty() && child != "/") {
        pathParts.push_back(child);
        fullPath = std::filesystem::path{parent};
        child = fullPath.filename().string();
        parent = fullPath.parent_path().string();
    }

    // traverse from starting point and find next path part
    while (!pathParts.empty()) {
        // get path part
        auto pathPart = pathParts.back();
        pathParts.pop_back();

        // searched current dir for dirEntry
        auto dirEntries = mFilesystem->read_dir_entry_as_dir(curDirEntry);
        auto searchedDirEntry = std::find_if(dirEntries.begin(), dirEntries.end(), [&pathPart](const DirEntry& dirEntry) {
            return Utils::remove_padding(dirEntry.mFilename) == pathPart;
        });

        // current path part not found -> end prematurely
        if (searchedDirEntry == std::end(dirEntries)) {
            std::cout << pathPart << " - not found" << std::endl;
            return std::nullopt;
        }
        // path part found -> keep going
        curDirEntry = *searchedDirEntry;
    }

    // found the correct dirEntry from path -> now check for filetype
    if (type == DirEntryType::BOTH) return curDirEntry;
    if (curDirEntry.mIsFile && type == DirEntryType::FILE) return curDirEntry;
    if (!curDirEntry.mIsFile && type == DirEntryType::DIR) return curDirEntry;

    auto str = (curDirEntry.mIsFile) ? "directory" : "file";   // reverse logic
    std::cout << curDirEntry.mFilename << " is not a " << str << std::endl;

    return std::nullopt;
}

void Shell::mount(const std::string &fsName) {
    mFilesystem = std::make_unique<Filesystem>(fsName);
    mFilesystem->mount();
}

void Shell::run(std::istream& istream) {
    bool ret = true;
    while(ret) {
        Arguments args;
        std::string command, opcode, arg;

        // prompt
        std::cout << std::endl << "root@root:" << mCWD << std::endl;
        std::cout << "$";
        std::getline(istream, command);
        if (Utils::is_white_space(command)) continue;

        // print out input if automated
        if (dynamic_cast<std::istringstream *>(&istream) != nullptr)
            std::cout << command << std::endl;

        // find space
        size_t pos = command.find_first_of(' ');
        if (pos != std::string::npos) {
            // if space, parse command and args
            opcode = command.substr(0, pos);

            std::stringstream stream{command.substr(pos + 1)};
            // load vector with parsed args
            while (std::getline(stream, arg, ' ')) {
                args.emplace_back(arg);
            }
        }
        // the opcode is the whole inputted line
        else opcode = command;

        // check if command exists
        if (!mHandlerMap.contains(opcode)) {
            std::cout << "Invalid command: " << opcode << std::endl;
            continue;
        }
        if (!check_args_count(opcode, args.size())) {
            std::cout << "Incorrect number of arguments" << std::endl;
            continue;
        }
        ret = mHandlerMap[opcode](args);
    }
}