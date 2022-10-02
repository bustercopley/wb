cmake_minimum_required(VERSION 3.14)

project(treeTests LANGUAGES CXX)

include(../cmake/project-is-top-level.cmake)
include(../cmake/folders.cmake)

# ---- Dependencies ----

if(PROJECT_IS_TOP_LEVEL)
  find_package(tree REQUIRED)
  enable_testing()
endif()

find_package(xoshiro256starstar REQUIRED)

# ---- Tests ----

add_executable(test_tree source/test_tree.cpp)
target_link_libraries(test_tree PRIVATE tree::tree)
target_link_libraries(test_tree PRIVATE xoshiro256starstar::xoshiro256starstar)
target_compile_features(test_tree PRIVATE cxx_std_20)

add_test(NAME test_tree COMMAND test_tree)

# ---- End-of-file commands ----

add_folders(Test)
