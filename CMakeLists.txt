cmake_minimum_required(VERSION 3.23)
project(KIV_ZOS)

set(CMAKE_CXX_STANDARD 20)

include_directories(sp)

add_executable(KIV_ZOS
        sp/src/Main.cpp)
