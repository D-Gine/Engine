#!/bin/bash
display_helper() {
    echo "To use this executer you must use a flag:
        --init                  Initialize vcpkg
        --build, -b             Build the program with CMake
        --style-check, -cs      Check for coding style using cpplint
        --help, -h              More information about this script

        << This section delete the old files created by the compilation >>
        --re-build, -rb         Build the program with CMake
        --debug-build, -d       Build the program with debug and verbose
        --clear, -c             Clear files created by the compilation and more
    "
}

init() {
    git clone https://github.com/microsoft/vcpkg.git
    export VCPKG_ROOT=./vcpkg
    export PATH=$VCPKG_ROOT:$PATH
    cd ./vcpkg && ./vcpkg/bootstrap-vcpkg.sh && cd ..
    cmake -B build -S .
    cmake --build build/
}

clear_project() {
    rm -rf ./build/
    rm -rf dengine
    rm -rf vcpkg/
    rm -rf .cache/
}

if [[ $1 == "--build" || $1 == "-b" ]]
then
    echo "------------BUILD------------"
    if [ ! -d "./build/" ]
    then
        mkdir ./build/ && cd ./build/
        cmake ..
        cd ..
    fi
    cmake --build ./build/ -j
    echo "------------END------------"

elif [[ $1 == "--re-build" || $1 == "-rb" ]]
then
    clear
    echo "------------RE-BUILD------------"
    clear_project
    mkdir ./build/ && cd ./build/
    cmake ..
    cmake --build . -j
    cd ..
    echo "------------END------------"

elif [[ $1 == "--debug-build" || $1 == "-d" ]]
then
    echo "------------DEBUG------------"
    clear_project
    mkdir ./build/ && cd ./build/
    cmake .. -DCMAKE_BUILD_TYPE=Debug
    cmake --build . -v -j
    cd ..
    echo "------------END------------"

elif [[ $1 == "--clear" || $1 == "-c" ]]
then
    clear_project

elif [[ $1 == "--style-check" || $1 == "-cs" ]]
then
    echo "------------CS CHECKER------------"
    pip install cpplint
    cpplint --recursive .
    echo "------------END------------"
elif [[ $1 == "--init" ]]
then
    init

elif [[ $1 == "--help" || $1 == "-h" ]]
then
    display_helper
else
    display_helper
fi
