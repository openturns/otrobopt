cmake_minimum_required (VERSION 3.18)

option (BUILD_PYTHON                 "Build the python module for the library"                               ON)
option (USE_SPHINX                   "Use sphinx for documentation"                                          OFF)
option (BUILD_SHARED_LIBS            "Build shared libraries"                                                ON)

# By default, build in Release mode. Must appear before project() command
set( CMAKE_BUILD_TYPE Release CACHE STRING "Build type")

project (otrobopt)

string (TOLOWER ${CMAKE_PROJECT_NAME} PACKAGE_NAME)

# set module dir to find custom scripts
list (APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

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

find_package (OpenTURNS CONFIG REQUIRED)
message (STATUS "Found OpenTURNS: ${OPENTURNS_ROOT_DIR} (found version \"${OPENTURNS_VERSION_STRING}\")")

if (NOT BUILD_SHARED_LIBS)
  list ( APPEND OTROBOPT_DEFINITIONS "-DOTROBOPT_STATIC" )
endif ()
add_definitions ( ${OTROBOPT_DEFINITIONS} )

if (MSVC)
  # Disable some warnings
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4197 /wd4244 /wd4251 /wd4267 /wd4996")
endif ()

include (GNUInstallDirs)

set (OTROBOPT_LIBRARY_PATH ${CMAKE_INSTALL_LIBDIR})
set (OTROBOPT_INCLUDE_PATH ${CMAKE_INSTALL_INCLUDEDIR})
set (OTROBOPT_SWIG_INCLUDE_DIRS ${OTROBOPT_INCLUDE_PATH}/${PACKAGE_NAME}/swig)
set (OTROBOPT_DATA_PATH ${CMAKE_INSTALL_DATAROOTDIR})
set (OTROBOPT_CONFIG_CMAKE_PATH ${CMAKE_INSTALL_LIBDIR}/cmake/otrobopt)
set (OTROBOPT_DOC_PATH ${CMAKE_INSTALL_DOCDIR})

if (BUILD_PYTHON)
  find_package (SWIG 4)
  include (${SWIG_USE_FILE})

  option (USE_PYTHON_SABI "Use Python stable ABI" OFF)
  if (USE_PYTHON_SABI AND CMAKE_VERSION VERSION_GREATER_EQUAL 3.26)
    find_package (Python 3.6 COMPONENTS Interpreter Development.SABIModule)
    add_library (Python::Module ALIAS Python::SABIModule)
  else ()
    find_package (Python 3.6 COMPONENTS Interpreter Development.Module)
  endif ()

  if (Python_FOUND)
    include (FindPythonModule)
    find_python_module (matplotlib)

    if (USE_SPHINX)
      find_program (SPHINX_EXECUTABLE NAMES sphinx-build DOC "Sphinx Documentation Builder (sphinx-doc.org)")
      find_python_module (numpydoc)
      find_python_module (sphinx_gallery)
      find_python_module (sphinx_copybutton)
      if (SPHINX_EXECUTABLE AND NUMPYDOC_FOUND AND SPHINX_GALLERY_FOUND AND SPHINX_COPYBUTTON_FOUND AND MATPLOTLIB_FOUND)
        find_python_module (sphinx)
      endif ()
      if (NOT SPHINX_FOUND)
        message (STATUS "Disabling documentation (need sphinx/pandoc/numpydoc/sphinx-gallery/matplotlib)")
      endif ()
    endif ()

    if (WIN32)
      set (OTROBOPT_PYTHON_MODULE_PATH Lib/site-packages CACHE STRING "site-packages dir")
    else ()
      set (OTROBOPT_PYTHON_MODULE_PATH ${CMAKE_INSTALL_LIBDIR}/python${Python_VERSION_MAJOR}.${Python_VERSION_MINOR}/site-packages CACHE STRING "site-packages dir")
    endif ()
  endif ()
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

# Some useful macros to ease CMakeLists.txt file writing
set (SOURCEFILES "" CACHE INTERNAL "List of source files to compile")
macro (ot_add_source_file FILENAME)
  set (sf ${SOURCEFILES} ${CMAKE_CURRENT_SOURCE_DIR}/${FILENAME})
  set (SOURCEFILES ${sf} CACHE INTERNAL "List of source files to compile")
endmacro ()

macro (ot_install_header_file FILENAME)
  install (FILES ${PACKAGE_NAME}/${FILENAME} DESTINATION ${OTROBOPT_INCLUDE_PATH}/${PACKAGE_NAME})
endmacro ()

macro (ot_install_swig_file FILENAME)
  install (FILES ${FILENAME} DESTINATION ${OTROBOPT_SWIG_INCLUDE_DIRS})
endmacro ()

set ( INTERNAL_INCLUDE_DIRS "" CACHE INTERNAL "List of directories with header files needed for build" )
macro ( ot_add_current_dir_to_include_dirs )
  set ( inc_dirs ${INTERNAL_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR} )
  set ( INTERNAL_INCLUDE_DIRS ${inc_dirs} CACHE INTERNAL "List of directories with header files needed for build" )
endmacro ( ot_add_current_dir_to_include_dirs )


set ( CPACK_PACKAGE_NAME          ${PACKAGE_NAME} )
set ( CPACK_PACKAGE_VERSION_MAJOR 0              )
set ( CPACK_PACKAGE_VERSION_MINOR 15              )
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

if (Python_FOUND AND SWIG_FOUND)
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
