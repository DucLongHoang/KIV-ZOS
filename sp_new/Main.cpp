#include <iostream>
#include "Shell.hpp"

int main(int argc, char** argv) {
    Shell sh("myFS.dat");
    sh.run(std::cin);

    return EXIT_SUCCESS;
}