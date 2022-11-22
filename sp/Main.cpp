#include <algorithm>
#include <filesystem>

#include "Shell.hpp"

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


void cmd_pwd(const std::vector<std::string>& args) {
    std::cout << std::filesystem::current_path() << std::endl;
}

void cmd_exit(const std::vector<std::string>& args) {
    exit(EXIT_SUCCESS);
}

std::unordered_map<std::string, std::function<void (const std::vector<std::string>& args)>> cmdMap{
    {"ls", cmd_ls},
    {"cat", cmd_cat},
    {"pwd", cmd_pwd},
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
    const auto function = cmdMap.find(cmd);

    // call function
    if (function != cmdMap.end()) {
        function->second(args);
    }
}


int main(int argc, char** argv) {
    /* if (argc != 2) {
        std::cout << "argc: " << argc << std::endl;
        return EXIT_FAILURE;
    }

    std::string fsName(argv[1]);
    std::shared_ptr<IFilesystem> fs = std::make_shared<FAT>(fsName);
    std::unique_ptr<Shell> shell = std::make_unique<Shell>(fs);
    shell->run(); */

    int a = sum_sizeof(5, 's', 5.4);

//    char a = '5';
    std::cout << "sizeof: " << a << std::endl;
    std::cout << zero_padded_string("HelloThere", 14) << std::endl;

	return EXIT_SUCCESS;
}