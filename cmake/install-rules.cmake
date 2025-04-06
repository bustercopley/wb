if(PROJECT_IS_TOP_LEVEL)
  set(CMAKE_INSTALL_INCLUDEDIR include/wbtree CACHE PATH "")
endif()

# Project is configured with no languages, so tell GNUInstallDirs the lib dir
set(CMAKE_INSTALL_LIBDIR lib CACHE PATH "")

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# find_package(<package>) call for consumers to find this project
set(package wbtree)

install(
    DIRECTORY include/
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    COMPONENT wbtree_Development
)

install(
    TARGETS wbtree_wbtree
    EXPORT wbtreeTargets
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

write_basic_package_version_file(
    "${package}ConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion
    ARCH_INDEPENDENT
)

# Allow package maintainers to freely override the path for the configs
set(
    wbtree_INSTALL_CMAKEDIR "${CMAKE_INSTALL_DATADIR}/${package}"
    CACHE PATH "CMake package config location relative to the install prefix"
)
mark_as_advanced(wbtree_INSTALL_CMAKEDIR)

install(
    FILES cmake/install-config.cmake
    DESTINATION "${wbtree_INSTALL_CMAKEDIR}"
    RENAME "${package}Config.cmake"
    COMPONENT wbtree_Development
)

install(
    FILES "${PROJECT_BINARY_DIR}/${package}ConfigVersion.cmake"
    DESTINATION "${wbtree_INSTALL_CMAKEDIR}"
    COMPONENT wbtree_Development
)

install(
    EXPORT wbtreeTargets
    NAMESPACE wbtree::
    DESTINATION "${wbtree_INSTALL_CMAKEDIR}"
    COMPONENT wbtree_Development
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
