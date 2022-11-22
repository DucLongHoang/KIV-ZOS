#include "Shell.hpp"

void Shell::run() {
    std::string input;
    while (true) {
        std::cout << ">";
        std::getline(std::cin, input);
        if (!is_white_space(input))
            process_input(input);
    }
}

void Shell::process_input(const std::string &input) {
    std::vector<std::string> args;
    std::stringstream stream{input};
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
        auto itr = cmdMap.find(cmdStr);
        if (itr == cmdMap.end() ){
            return CMD::UNKNOWN;
        }
        return itr->second;
    };
    const CMD CMD = mapCmd(cmd);
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

bool Shell::cp(const std::vector<std::string>& args) {
    mFS->fs_write(args);
    std::cout << "Writting..." << std::endl;
    return false;
}

bool Shell::mv(const std::vector<std::string>& args) {

    return false;
}

bool Shell::rm(const std::vector<std::string>& args) {


    return false;
}

bool Shell::mkdir(const std::vector<std::string>& args) {

    return false;
}

bool Shell::rmdir(const std::vector<std::string>& args) {

    return false;
}

bool Shell::ls(const std::vector<std::string>& args) {

    return false;
}

bool Shell::cat(const std::vector<std::string>& args) {

    return false;
}

bool Shell::cd(const std::vector<std::string> &args) {

    return false;
}

bool Shell::pwd(const std::vector<std::string>& args) {

    return false;
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


    return false;
}

bool Shell::xcp(const std::vector<std::string>& args) {

    return false;
}

bool Shell::exit(const std::vector<std::string>& args) {
    std::exit(EXIT_FAILURE);
    return false;
}

bool Shell::unknown(const std::vector<std::string> &args) {
    std::cout << "Unknown command" << std::endl;
    return false;
}
