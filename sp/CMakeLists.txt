cmake_minimum_required(VERSION 3.23)
project(KIV_ZOS)

set(CMAKE_CXX_STANDARD 20)

add_executable(sp
        Lib.hpp Lib.cpp
        Main.cpp)