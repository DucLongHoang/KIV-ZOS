#include <ranges>

#include "Shell.hpp"

Shell::Shell(const std::string& fsName) : mFsName(fsName), mCWD("/") {
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

void Shell::mount_fs(const std::string &fsName) {
    std::vector<std::any> tmp{};
    mFilesystem = std::make_unique<FAT_Filesystem>(fsName);
    mFilesystem->mount_fs(tmp);
}

[[noreturn]] void Shell::run() {
    std::string input;
    while (true) {
        std::cout << mCWD << ">";
        std::getline(std::cin, input);
        if (!is_white_space(input))
            process_input(input);
    }
}

void Shell::process_input(const std::string &input) {
    std::stringstream stream{input};
    std::vector<std::string> args;
    std::string cmd;

    // get cmd
    std::getline(stream, cmd, ' ');

    // load vector with parsed args
    std::string token;
    while (std::getline(stream, token, ' ')) {
        args.emplace_back(token);
    }

    // find CMD by string
    auto mapCmd = [this](const std::string& cmdStr) -> Shell::CMD {
        if (cmdMap.contains(cmdStr))
            return cmdMap.find(cmdStr)->second;
        return CMD::UNKNOWN;
    };

    auto CMD = mapCmd(cmd);
    execute_cmd(CMD, args);
}

void Shell::execute_cmd(const CMD& CMD, const std::vector<std::string>& args) {
    switch (CMD) {
        case CMD::CP: cp(args); break;
        case CMD::MV: mv(args); break;
        case CMD::RM: rm(args);break;
        case CMD::MKDIR: mkdir(args); break;
        case CMD::RMDIR: rmdir(args); break;
        case CMD::LS: ls(args); break;
        case CMD::CAT: cat(args); break;
        case CMD::CD: cd(args); break;
        case CMD::PWD: pwd(args); break;
        case CMD::INFO: info(args); break;
        case CMD::INCP: incp(args); break;
        case CMD::OUTCP: outcp(args); break;
        case CMD::LOAD: load(args); break;
        case CMD::FORMAT: format(args); break;
        case CMD::XCP: xcp(args); break;
        case CMD::EXIT: exit(args); break;
        case CMD::UNKNOWN: unknown(args); break;
    }
}

bool Shell::check_argc(const std::vector<std::string>& args, uint needed) const {
    if (args.size() != needed) {
        std::cout << "Incorrect number of arguments!" << std::endl;
        std::cout << "Needed: " << needed << std::endl;
        std::cout << "Found:  " << args.size() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

bool Shell::cp(const std::vector<std::string>& args) {
//    mFilesystem->fs_write(args);
    return true;
}

bool Shell::mv(const std::vector<std::string>& args) {

    return false;
}

bool Shell::rm(const std::vector<std::string>& args) {


    return false;
}

bool Shell::mkdir(const std::vector<std::string>& args) {
    if (check_argc(args, 1) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    auto cwd = mCWD;
    auto dirName = args.front();


    return EXIT_SUCCESS;
}

bool Shell::rmdir(const std::vector<std::string>& args) {

    return false;
}

bool Shell::ls(const std::vector<std::string>& args) {
    if (check_argc(args, 0) == EXIT_FAILURE && check_argc(args, 1) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }
    std::vector<std::any> castedArgs(args.begin(), args.end());
    uint fd = mFilesystem->fs_open(castedArgs);
    std::vector<char> tmpBuf(CLUSTER_SIZE);

    castedArgs.emplace_back(fd);
    castedArgs.emplace_back(std::move(tmpBuf));
    mFilesystem->fs_read(castedArgs);

    auto& buffer = any_cast<std::vector<char>&>(castedArgs[1]);
    std::cout << "Listing directory content: ";

    std::stringstream ss;
    for (auto i : buffer) {
        write_to_stream(ss, i);
    }

    std::vector<std::string> dirItems{};
    DirectoryItem di;
    while(true) {
        di.mFilename = string_from_stream(ss, FILENAME_LEN);
        read_from_stream(ss, di.mIsFile);
        read_from_stream(ss, di.mSize);
        read_from_stream(ss, di.mStartCluster);
        std::string empty{};
        empty.resize(FILENAME_LEN, '\0');
        if (di.mFilename == empty) break;
        else dirItems.emplace_back(di.mFilename);
    }

    auto filesInFolder = dirItems | std::views::drop(1);
    for (const auto& str : filesInFolder) {
        std::cout << str.c_str() << "\t";
    }
    std::cout << std::endl;

    return EXIT_SUCCESS;
}

bool Shell::cat(const std::vector<std::string>& args) {
    if (check_argc(args, 1) == EXIT_FAILURE)
        return EXIT_FAILURE;

    std::vector<std::any>castedArgs{args.front()};
    // file descriptor is cluster number
    auto fd = mFilesystem->fs_open(castedArgs);
    castedArgs.clear();

    // file descriptor, string buffer and size
    castedArgs.emplace_back(fd);
    castedArgs.emplace_back(std::string{});
    castedArgs.emplace_back(/*some buffer size*/);
    mFilesystem->fs_read(castedArgs);

    std::cout << "File content: \n" << any_cast<std::string>(castedArgs[1]) << std::endl;
    return EXIT_SUCCESS;
}

bool Shell::cd(const std::vector<std::string> &args) {

    return false;
}

bool Shell::pwd(const std::vector<std::string>& args) {
    if (check_argc(args, 0) == EXIT_FAILURE)
        return EXIT_FAILURE;

    std::cout << "Current working directory: " << mCWD << std::endl;
    return EXIT_SUCCESS;
}

bool Shell::info(const std::vector<std::string>& args) {

    return false;
}

bool Shell::incp(const std::vector<std::string>& args) {

    return false;
}

bool Shell::outcp(const std::vector<std::string>& args) {

    return false;
}

bool Shell::load(const std::vector<std::string>& args) {

    return false;
}

bool Shell::format(const std::vector<std::string>& args) {
    if (check_argc(args, 2) == EXIT_FAILURE)
        return EXIT_FAILURE;

    std::string msg = (std::filesystem::exists(mFsName)) ?
            "Formatting existing disk..." : "Creating new disk...";
    std::cout << msg << std::endl;
    mFilesystem = std::make_unique<FAT_Filesystem>(mFsName);

    std::vector<std::any> castedArgs(args.begin(), args.end());
    return mFilesystem->init_fs(castedArgs);
}

bool Shell::xcp(const std::vector<std::string>& args) {

    return false;
}

bool Shell::exit(const std::vector<std::string>& args) {
    std::exit(EXIT_SUCCESS);
    return EXIT_SUCCESS;
}

bool Shell::unknown(const std::vector<std::string> &args) {
    std::cout << "Unknown command" << std::endl;
    return false;
}

