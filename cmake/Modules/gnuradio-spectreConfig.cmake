find_package(PkgConfig)

PKG_CHECK_MODULES(PC_GR_SPECTRE gnuradio-spectre)

FIND_PATH(
    GR_SPECTRE_INCLUDE_DIRS
    NAMES gnuradio/spectre/api.h
    HINTS $ENV{SPECTRE_DIR}/include
        ${PC_SPECTRE_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    GR_SPECTRE_LIBRARIES
    NAMES gnuradio-spectre
    HINTS $ENV{SPECTRE_DIR}/lib
        ${PC_SPECTRE_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
          )

include("${CMAKE_CURRENT_LIST_DIR}/gnuradio-spectreTarget.cmake")

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GR_SPECTRE DEFAULT_MSG GR_SPECTRE_LIBRARIES GR_SPECTRE_INCLUDE_DIRS)
MARK_AS_ADVANCED(GR_SPECTRE_LIBRARIES GR_SPECTRE_INCLUDE_DIRS)
