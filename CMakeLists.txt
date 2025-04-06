cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    wbtree
    VERSION 0.1.0
    DESCRIPTION "Weight-balanced binary tree"
    HOMEPAGE_URL "https://example.com/"
    LANGUAGES NONE
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# ---- Declare library ----

add_library(wbtree_wbtree INTERFACE)
add_library(wbtree::wbtree ALIAS wbtree_wbtree)

set_property(
    TARGET wbtree_wbtree PROPERTY
    EXPORT_NAME wbtree
)

target_include_directories(
    wbtree_wbtree ${warning_guard}
    INTERFACE
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)

target_compile_features(wbtree_wbtree INTERFACE cxx_std_20)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----

if(NOT wbtree_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of wbtree"
  )
endif()

include(cmake/dev-mode.cmake)
