cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    tree
    VERSION 0.1.0
    DESCRIPTION "Weight-balanced binary tree"
    HOMEPAGE_URL "https://example.com/"
    LANGUAGES NONE
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# ---- Declare library ----

add_library(tree_tree INTERFACE)
add_library(tree::tree ALIAS tree_tree)

set_property(
    TARGET tree_tree PROPERTY
    EXPORT_NAME tree
)

target_include_directories(
    tree_tree ${warning_guard}
    INTERFACE
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)

target_compile_features(tree_tree INTERFACE cxx_std_20)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----

if(NOT tree_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of tree"
  )
endif()

include(cmake/dev-mode.cmake)
