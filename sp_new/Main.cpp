#include <iostream>
#include "Shell.hpp"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "No. of req. arguments:  2" << std::endl;
        std::cout << "No. of found arguments: " << argc << std::endl;
        return EXIT_FAILURE;
    }
    Shell sh(argv[1]);
    sh.run(std::cin);

    return EXIT_SUCCESS;
}