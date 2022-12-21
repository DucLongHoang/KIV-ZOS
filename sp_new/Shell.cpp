#include "Shell.hpp"

#include <filesystem>

Shell::Shell(const std::string& fsName) : mFsName(fsName), mCWD("/") {
    fill_handlers();

    if (std::filesystem::exists(fsName)) {
        std::cout << "File system: " << fsName << " found" << std::endl;
        std::cout << "Mounting file system..." << std::endl;
        mount_fs(fsName);
    }
    else {
        std::cout << "File system: " << fsName << " not found" << std::endl;
        std::cout << "Create new disk by using cmd: 'format [x] [y]'" << std::endl;
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
        return true;
    };
    mHandlerMap["xcp"] = [this](Arguments& args) -> bool {
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
    mArgsCountMap["cp"] = 2;
    mArgsCountMap["mv"] = 2;
    mArgsCountMap["rm"] = 1;
    mArgsCountMap["mkdir"] = 1;
    mArgsCountMap["rmdir"] = 1;
    mArgsCountMap["ls"] =
    mArgsCountMap["cat"] =
    mArgsCountMap["cd"] =
    mArgsCountMap["pwd"] =
    mArgsCountMap["info"] =
    mArgsCountMap["incp"] =
    mArgsCountMap["outcp"] =
    mArgsCountMap["load"] =
    mArgsCountMap["format"] =
    mArgsCountMap["xcp"] =
    mArgsCountMap["exit"] =
    mArgsCountMap["quit"] =
    mArgsCountMap["close"] =
}

void Shell::run(std::istream& istream) {
    Arguments args;
    std::string command, opcode, arg;
    bool ret = true;

    while(ret) {
        // prompt
        std::cout << mCWD << "$ ";
        std::getline(istream, command);

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
        ret = mHandlerMap[opcode](args);
    }
}