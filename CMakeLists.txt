cmake_minimum_required (VERSION 3.2)

option (BUILD_PYTHON                 "Build the python module for the library"                               ON)
option (USE_SPHINX                   "Use sphinx for documentation"                                          ON)
option (BUILD_SHARED_LIBS            "Build shared libraries"                                                ON)

# By default, build in Release mode. Must appear before project() command
if ( NOT DEFINED CMAKE_BUILD_TYPE )
   set( CMAKE_BUILD_TYPE Release CACHE STRING "Build type")
endif ()

project (OTRobOpt)

string ( TOLOWER ${CMAKE_PROJECT_NAME} PACKAGE_NAME )

# set module dir to find custom scripts
list ( APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake )

find_package (OpenTURNS CONFIG REQUIRED)
message (STATUS "Found OpenTURNS: ${OPENTURNS_ROOT_DIR} (found version \"${OPENTURNS_VERSION_STRING}\")")
if (NOT DEFINED OPENTURNS_PYTHON_MODULE_PATH)
  set (OPENTURNS_PYTHON_MODULE_PATH ${OPENTURNS_PYTHON3_MODULE_PATH})
endif ()

if (NOT BUILD_SHARED_LIBS)
  list ( APPEND OTROBOPT_DEFINITIONS "-DOTROBOPT_STATIC" )
endif ()
add_definitions ( ${OTROBOPT_DEFINITIONS} )

if (NOT DEFINED CMAKE_INSTALL_LIBDIR)
  set (CMAKE_INSTALL_LIBDIR lib${LIB_SUFFIX})
endif ()

set (OTROBOPT_LIBRARY_PATH ${CMAKE_INSTALL_LIBDIR})
set (OTROBOPT_INCLUDE_PATH include)
set (OTROBOPT_SWIG_INCLUDE_DIRS ${OTROBOPT_INCLUDE_PATH}/${PACKAGE_NAME}/swig)
set (OTROBOPT_DATA_PATH share)
set (OTROBOPT_CONFIG_CMAKE_PATH ${CMAKE_INSTALL_LIBDIR}/cmake/otrobopt)
set (OTROBOPT_DOC_PATH share/${PACKAGE_NAME}/doc)

set (CMAKE_CXX_STANDARD 11)

if (BUILD_PYTHON)
  find_package (SWIG 3)

  find_package (PythonInterp 3)
  if (PYTHONINTERP_FOUND)
    find_package (PythonLibs ${PYTHON_VERSION_STRING} EXACT)
    include (FindPythonModule)

    if (USE_SPHINX)
      find_program (SPHINX_EXECUTABLE NAMES sphinx-build DOC "Sphinx Documentation Builder (sphinx-doc.org)")
      find_program (DVISVGM_EXECUTABLE NAMES dvisvgm DOC "dvi to svg converter")
      find_program (DVIPNG_EXECUTABLE NAMES dvipng DOC "dvi to png converter")
      if (NOT (DVISVGM_EXECUTABLE OR DVIPNG_EXECUTABLE))
        message (STATUS "Could NOT find dvisvgm|dvipng executable")
      endif ()
      if (SPHINX_EXECUTABLE AND (DVISVGM_EXECUTABLE OR DVIPNG_EXECUTABLE))
        find_python_module (sphinx 1.8)
        find_python_module (numpydoc 0.9)
      endif ()
      if (NOT SPHINX_FOUND OR NOT NUMPYDOC_FOUND)
        message (STATUS "Disabling documentation (need dvisvgm/sphinx/pandoc/numpydoc)")
      endif ()
    endif ()

    if (DEFINED PYTHON_SITE_PACKAGES)
      set (OTROBOPT_PYTHON_MODULE_PATH "${PYTHON_SITE_PACKAGES}")
    else ()
     if (WIN32)
       set (OTROBOPT_PYTHON_MODULE_PATH Lib/site-packages)
     else ()
       set (OTROBOPT_PYTHON_MODULE_PATH ${CMAKE_INSTALL_LIBDIR}/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/site-packages)
      endif ()
    endif ()

    set (OTROBOPT_PYTHON${PYTHON_VERSION_MAJOR}_MODULE_PATH ${OTROBOPT_PYTHON_MODULE_PATH})
  endif ( PYTHONINTERP_FOUND )
endif ()


# Define variables without OTROBOPT_ prefix with absolute paths
foreach (_var PYTHON_MODULE_PATH LIBRARY_PATH INCLUDE_PATH CONFIG_CMAKE_PATH)
  if (IS_ABSOLUTE "${OTROBOPT_${_var}}")
    set (${_var} ${OTROBOPT_${_var}})
  else ()
    set (${_var} ${CMAKE_INSTALL_PREFIX}/${OTROBOPT_${_var}})
  endif ()
  get_filename_component (${_var} "${${_var}}" ABSOLUTE)
  file (TO_NATIVE_PATH "${${_var}}" NATIVE_${_var})
  string (REPLACE "\\" "\\\\" NATIVE_${_var} ${NATIVE_${_var}})
endforeach ()

# RPATH settings
set (CMAKE_INSTALL_RPATH ${LIBRARY_PATH})
set (CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
if (POLICY CMP0042)
  # MACOSX_RPATH is enabled by default
  cmake_policy(SET CMP0042 NEW)
endif()
if (POLICY CMP0078)
  # UseSWIG generates standard target names
  cmake_policy(SET CMP0078 NEW)
endif()
if (POLICY CMP0086)
  # UseSWIG honors SWIG_MODULE_NAME via -module flag
  cmake_policy(SET CMP0086 NEW)
endif()

# Some useful macros to ease CMakeLists.txt file writing
set ( SOURCEFILES "" CACHE INTERNAL "List of source files to compile" )
macro ( ot_add_source_file FILENAME )
  set ( sf ${SOURCEFILES} ${CMAKE_CURRENT_SOURCE_DIR}/${FILENAME} )
  set ( SOURCEFILES ${sf} CACHE INTERNAL "List of source files to compile" )
endmacro ( ot_add_source_file )

set ( HEADERFILES "" CACHE INTERNAL "List of header files to install" )
macro ( ot_install_header_file FILENAME )
  set ( hf ${HEADERFILES} ${CMAKE_CURRENT_SOURCE_DIR}/${PACKAGE_NAME}/${FILENAME} )
  set ( HEADERFILES ${hf} CACHE INTERNAL "List of header files to install" )
endmacro ( ot_install_header_file )

set ( SWIGFILES "" CACHE INTERNAL "List of SWIG files to install" )
macro ( ot_install_swig_file FILENAME )
  set ( hf ${SWIGFILES} ${CMAKE_CURRENT_SOURCE_DIR}/${FILENAME} )
  set ( SWIGFILES ${hf} CACHE INTERNAL "List of SWIG files to install" )
endmacro ( ot_install_swig_file )

set ( INTERNAL_INCLUDE_DIRS "" CACHE INTERNAL "List of directories with header files needed for build" )
macro ( ot_add_current_dir_to_include_dirs )
  set ( inc_dirs ${INTERNAL_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR} )
  set ( INTERNAL_INCLUDE_DIRS ${inc_dirs} CACHE INTERNAL "List of directories with header files needed for build" )
endmacro ( ot_add_current_dir_to_include_dirs )


set ( CPACK_PACKAGE_NAME          ${PACKAGE_NAME} )
set ( CPACK_PACKAGE_VERSION_MAJOR 0              )
set ( CPACK_PACKAGE_VERSION_MINOR 11              )
set ( CPACK_PACKAGE_VERSION_PATCH                )
set ( CPACK_SOURCE_GENERATOR      "TGZ;TBZ2"     )
set (CPACK_BINARY_STGZ    OFF CACHE BOOL "STGZ")
set (CPACK_BINARY_TBZ2    ON  CACHE BOOL "TBZ2")
set (CPACK_BINARY_TGZ     ON  CACHE BOOL "TGZ" )
set (CPACK_BINARY_TZ      OFF CACHE BOOL "TZ"  )
set ( CPACK_SOURCE_IGNORE_FILES "/.git;/build;.*~;${CPACK_SOURCE_IGNORE_FILES}" )
set ( PACKAGE_NAME           ${CPACK_PACKAGE_NAME} )
set ( PACKAGE_VERSION        ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR} )
if ( CPACK_PACKAGE_VERSION_PATCH )
  set ( PACKAGE_VERSION       ${PACKAGE_VERSION}.${CPACK_PACKAGE_VERSION_PATCH} )
endif ()
set ( CPACK_SOURCE_PACKAGE_FILE_NAME ${PACKAGE_NAME}-${PACKAGE_VERSION} )


# The tests can't be run if this function is absent
enable_testing ()
add_custom_target (tests COMMENT "Build tests")
add_custom_target (check COMMENT "Run pre-installation tests")
add_custom_target (installcheck COMMENT "Run post-installation tests")

add_subdirectory (lib)
add_dependencies (check cppcheck)

if (PYTHONINTERP_FOUND AND PYTHONLIBS_FOUND AND SWIG_FOUND)
  add_subdirectory (python)
  add_dependencies (installcheck pyinstallcheck)
endif ()


# uninstall target
configure_file (
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
  IMMEDIATE @ONLY
)

add_custom_target ( uninstall
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
)

set ( OTROBOPT_LIBRARY                 otrobopt )
set ( OTROBOPT_LIBRARIES ${OPENTURNS_LIBRARIES} )
list ( APPEND OTROBOPT_LIBRARIES ${OTROBOPT_LIBRARY} )
set ( OTROBOPT_LIBRARY_DIRS ${OPENTURNS_LIBRARY_DIRS} )
list ( APPEND OTROBOPT_LIBRARY_DIRS ${LIBRARY_PATH} )
set ( OTROBOPT_INCLUDE_DIR ${INCLUDE_PATH} )
set ( OTROBOPT_INCLUDE_DIRS ${OTROBOPT_INCLUDE_DIR} )
set ( OTROBOPT_ROOT_DIR       ${CMAKE_INSTALL_PREFIX} )
list ( APPEND OTROBOPT_INCLUDE_DIRS ${OPENTURNS_INCLUDE_DIRS} )
set ( OTROBOPT_VERSION_STRING ${PACKAGE_VERSION} )
set ( OTROBOPT_VERSION_MAJOR ${CPACK_PACKAGE_VERSION_MAJOR} )
set ( OTROBOPT_VERSION_MINOR ${CPACK_PACKAGE_VERSION_MINOR} )
set ( OTROBOPT_VERSION_PATCH ${CPACK_PACKAGE_VERSION_PATCH} )

include(CMakePackageConfigHelpers)
configure_package_config_file (cmake/OTRobOptConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/OTRobOptConfig.cmake
  INSTALL_DESTINATION ${OTROBOPT_CONFIG_CMAKE_PATH}
  PATH_VARS
    OTROBOPT_ROOT_DIR
    OTROBOPT_INCLUDE_DIR
    OTROBOPT_INCLUDE_DIRS
    OTROBOPT_LIBRARY_DIRS
    PYTHON_MODULE_PATH
  NO_CHECK_REQUIRED_COMPONENTS_MACRO)

write_basic_package_version_file (OTRobOptConfigVersion.cmake VERSION ${OTROBOPT_VERSION_STRING} COMPATIBILITY AnyNewerVersion)

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/OTRobOptConfig.cmake
               ${CMAKE_CURRENT_BINARY_DIR}/OTRobOptConfigVersion.cmake
         DESTINATION ${OTROBOPT_CONFIG_CMAKE_PATH})

include (CPack)
