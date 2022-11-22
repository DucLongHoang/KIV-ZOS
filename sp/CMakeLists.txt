cmake_minimum_required(VERSION 3.23)

add_executable(sp
        utils.hpp utils.cpp
        Shell.hpp Shell.cpp
        Filesystem.hpp Filesystem.cpp
        Main.cpp)