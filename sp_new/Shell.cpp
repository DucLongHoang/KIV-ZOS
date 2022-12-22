#include "Shell.hpp"

#include <regex>
#include <filesystem>
#include <string_view>

static const std::regex REGEX_FORMAT("[1-9]+[0-9]*(kb|Kb|KB|mb|Mb|MB)");
static const std::regex REGEX_KB("(kb|Kb|KB){1}");

Shell::Shell(const std::string& fsName) : mFsName(fsName), mCWD("/") {
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
        return true;
    };
    mHandlerMap["mv"] = [this](Arguments& args) -> bool {
        return true;
    };
    mHandlerMap["rm"] = [this](Arguments& args) -> bool {
        return true;
    };
    mHandlerMap["mkdir"] = [this](Arguments& args) -> bool {
        return true;
    };
    mHandlerMap["rmdir"] = [this](Arguments& args) -> bool {
        return true;
    };
    mHandlerMap["ls"] = [this](Arguments& args) -> bool {
        return true;
    };
    mHandlerMap["cat"] = [this](Arguments& args) -> bool {
        return true;
    };
    mHandlerMap["cd"] = [this](Arguments& args) -> bool {
        return true;
    };
    mHandlerMap["pwd"] = [this](Arguments& args) -> bool {
        return true;
    };
    mHandlerMap["info"] = [this](Arguments& args) -> bool {
        return true;
    };
    mHandlerMap["incp"] = [this](Arguments& args) -> bool {
        return true;
    };
    mHandlerMap["outcp"] = [this](Arguments& args) -> bool {
        return true;
    };
    mHandlerMap["load"] = [this](Arguments& args) -> bool {
        return true;
    };
    mHandlerMap["format"] = [this](Arguments& args) -> bool {
        if (!std::regex_match(args[0], REGEX_FORMAT)) {
            std::cout << "Invalid input: " << args[0] << std::endl;
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

        return true;
    };
    mHandlerMap["xcp"] = [this](Arguments& args) -> bool {
        return true;
    };
    mHandlerMap["short"] = [this](Arguments& args) -> bool {
        return true;
    };
    mHandlerMap["exit"] = [this](Arguments& args) -> bool {
        return false;
    };
    mHandlerMap["quit"] = [this](Arguments& args) -> bool {
        return false;
    };
    mHandlerMap["close"] = [this](Arguments& args) -> bool {
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

void Shell::mount(const std::string &fsName) {
    mFilesystem = std::make_unique<Filesystem>(fsName);
    mFilesystem->mount();
}

void Shell::run(std::istream& istream) {
    Arguments args;
    std::string command, opcode, arg;
    bool ret = true;

    while(ret) {
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
        if (check_args_count(opcode, args.size())) {
            std::cout << "Incorrect number of arguments" << std::endl;
            continue;
        }
        ret = mHandlerMap[opcode](args);
    }
}