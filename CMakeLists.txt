cmake_minimum_required(VERSION 3.10)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "Vcpkg toolchain file")

project(dengine)

########## SETUP ##########
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(SRC_DIR "${PROJECT_SOURCE_DIR}/src")
set(HDR_DIR "${PROJECT_SOURCE_DIR}/include")
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O2")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)

find_package(Lua REQUIRED)

include(FetchContent)

FetchContent_Declare(
    LuaCPP
    GIT_REPOSITORY https://github.com/jordanvrtanoski/luacpp
    GIT_TAG main
    SOURCE_SUBDIR Source
    EXCLUDE_FROM_ALL
)

FetchContent_MakeAvailable(LuaCPP)

add_subdirectory(ECS)


########## COMMUN ##########
add_executable(${PROJECT_NAME}
    ${SRC_DIR}/Entity.cpp
    ${SRC_DIR}/Server.cpp
    ${SRC_DIR}/Stat.cpp
    ${SRC_DIR}/main.cpp
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${HDR_DIR}
        ${luacpp_SOURCE_DIR}/Source
        ${LUA_INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        luacpp
        ${LUA_LIBRARIES}
        ${JSONCPP_LIBRARIES}
        ECS
)
