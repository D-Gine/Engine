cmake_minimum_required(VERSION 3.10)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "Vcpkg toolchain file")

project(dengine)

########## OPTIONS ##########
option(ENABLE_ECS_TESTS "Build tests along with the library" OFF)
option(ENABLE_ECS_COVERAGE "Enable coverage reporting" OFF)

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

find_package(jsoncpp CONFIG REQUIRED)

include(FetchContent)

########## TESTING ##########
if(ENABLE_ECS_COVERAGE)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -fprofile-arcs -ftest-coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
  endif()
endif()

if (ENABLE_ECS_TESTS)
  enable_testing()
  add_subdirectory(tests/unit_tests)
endif ()

########## COMMUN ##########
add_executable(${PROJECT_NAME}
    ${SRC_DIR}/Server.cpp
    ${SRC_DIR}/main.cpp

    # ECS
    ${SRC_DIR}/ECS/Registry.cpp
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${HDR_DIR}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        JsonCpp::JsonCpp
)
