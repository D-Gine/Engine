cmake_minimum_required(VERSION 3.10)
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

########## PACKAGES ##########
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)

########## COMMUN ##########
add_executable(${PROJECT_NAME}
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/Stat.cpp
    ${SRC_DIR}/Entity.cpp
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${HDR_DIR}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${JSONCPP_LIBRARIES}
)
