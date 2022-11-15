#include "Shell.hpp"

void Shell::run() {
    std::string input;
    while (true) {
        std::cout << ">";
        std::cin >> input;
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

    // find function
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
        case CMD::CP: {
            
            break;
        }
        case CMD::MV: {
            
            break;
        }
        case CMD::RM: {
            
            break;
        }
        case CMD::MKDIR: {
            
            break;
        }
        case CMD::RMDIR: {
            
            break;
        }
        case CMD::LS: {
            
            break;
        }
        case CMD::CAT: {
            
            break;
        }
        case CMD::CD: {
            
            break;
        }
        case CMD::PWD: {
            
            break;
        }
        case CMD::INFO: {
            
            break;
        }
        case CMD::INCP: {
            
            break;
        }
        case CMD::OUTCP: {
            
            break;
        }
        case CMD::LOAD: {
            
            break;
        }
        case CMD::FORMAT: {
            
            break;
        }
        case CMD::XCP: {
            
            break;
        }
        case CMD::EXIT: {
            
            break;
        }
        case CMD::UNKNOWN: {
            
            break;
        }
    }
}

bool Shell::cp(const std::vector<std::string>& args) {

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

    return false;
}