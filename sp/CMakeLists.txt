cmake_minimum_required(VERSION 3.23)

add_executable(sp
        Initializable.hpp
        utils.hpp utils.cpp
        IFilesystem.hpp
        FAT.hpp FAT.cpp
        Shell.hpp Shell.cpp
        Main.cpp)