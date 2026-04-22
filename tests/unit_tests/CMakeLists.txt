project(unit_tests)

########## GOOGLE TESTING ##########
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)

########## LINKAGE ##########
add_executable(${PROJECT_NAME}
  ECS/registry.cpp
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    GTest::gtest_main
)

target_include_directories(${PROJECT_NAME}
  PRIVATE
    ${HDR_DIR}
)

########## TESTS ##########
include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME}
  DISCOVERY_MODE PRE_TEST
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
