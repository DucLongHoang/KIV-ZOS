#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <map>
#include <filesystem>
#include <fstream>
#include "Lib.hpp"


void cmd_cp(const std::vector<std::string>& args) {

}

void cmd_mv(const std::vector<std::string>& args) {

}

void cmd_rm(const std::vector<std::string>& args) {

}

void cmd_mkdir(const std::vector<std::string>& args) {

}

void cmd_rmdir(const std::vector<std::string>& args) {

}

void cmd_ls(const std::vector<std::string>& args) {
    std::string currPath = std::filesystem::current_path().string();
    std::filesystem::directory_iterator dirIt {currPath};

    for (const auto& entry : dirIt) {
        std::cout << entry.path().filename() << std::endl;
    }
}

void cmd_cat(const std::vector<std::string>& args) {
    std::ifstream f(args[0]);
    std::ostringstream content("", std::ios::app);

    std::string line;
    while (std::getline(f, line)) {
        content << line << std::endl;
    }

    std::cout << content.str();
}

void cmd_cd(const std::vector<std::string>& args) {

}

void cmd_pwd(const std::vector<std::string>& args) {
    std::cout << std::filesystem::current_path() << std::endl;
}

void cmd_info(const std::vector<std::string>& args) {

}

void cmd_incp(const std::vector<std::string>& args) {

}

void cmd_outcp(const std::vector<std::string>& args) {

}

void cmd_load(const std::vector<std::string>& args) {

}

void cmd_format(const std::vector<std::string>& args) {

}

void cmd_xcp(const std::vector<std::string>& args) {

}

void cmd_exit(const std::vector<std::string>& args) {
    exit(EXIT_SUCCESS);
}

std::map<std::string, std::function<void (const std::vector<std::string>& args)>> cmdMap{
    {"cp", cmd_cp},
    {"mv", cmd_mv},
    {"rm", cmd_rm},
    {"mkdir", cmd_mkdir},
    {"rmdir", cmd_rmdir},
    {"ls", cmd_ls},
    {"cat", cmd_cat},
    {"cd", cmd_cd},
    {"pwd", cmd_pwd},
    {"info", cmd_info},
    {"incp", cmd_incp},
    {"outcp", cmd_outcp},
    {"load", cmd_load},
    {"format", cmd_format},
    {"xcp", cmd_xcp},
    {"exit", cmd_exit},
};

void processInput(const std::string& input) {
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
    auto function = cmdMap.find(cmd);

    // call function
    if (function != cmdMap.end()) {
        function->second(args);
    }
}

int main(int argc, char** argv) {

    std::string input;
    Lib lib(5, 9);

    while(true) {
        std::cout << lib.getSum();
        std::cout << ">";
        std::getline(std::cin, input);
        processInput(input);
    }


    return EXIT_SUCCESS;
}