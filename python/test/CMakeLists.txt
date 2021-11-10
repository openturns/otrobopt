

set (PATH_SEP ":")
if (WIN32)
  set (PATH_SEP "\\;")
endif ()


set ( PYINSTALLCHECK_ENVIRONMENT "OPENTURNS_CONFIG_PATH=${INSTALL_DESTDIR}${SYSCONFIG_PATH}/openturns"
                                 "PYTHONPATH=${OPENTURNS_PYTHON${PYTHON_VERSION_MAJOR}_MODULE_PATH}${PATH_SEP}${INSTALL_DESTDIR}${PYTHON_MODULE_PATH}${PATH_SEP}$ENV{PYTHONPATH}"
    )


set ( PYINSTALLCHECK_TO_BE_RUN )
macro ( ot_pyinstallcheck_test TESTNAME )
  set ( TESTNAME_TGT t_${TESTNAME} )
  set ( TESTNAME_SRC ${TESTNAME_TGT}.py )
  set ( TESTNAME_LOC ${CMAKE_CURRENT_SOURCE_DIR}/${TESTNAME_SRC})
  set ( _CMD )
  set ( _PRE )
  set ( _POST )
  set ( _PARAMS )
  set ( _IGNOREOUT OFF )
  foreach ( _ARG ${ARGN} )
    string ( TOUPPER ${_ARG} ARG )
    if ( ${ARG} MATCHES PRE )
      set ( _CMD PRE )
    elseif ( ${ARG} MATCHES POST )
      set ( _CMD POST )
    elseif ( ${ARG} MATCHES PARAMS )
      set ( _CMD PARAMS )
    elseif ( ${ARG} MATCHES IGNOREOUT )
      set ( _IGNOREOUT ON )
    else ( ${ARG} MATCHES PRE )
      if ( ${_CMD} MATCHES PRE )
        set ( _PRE "${_PRE} ${_ARG} && " )
      elseif ( ${_CMD} MATCHES POST )
        set ( _POST "${_POST} && ${_ARG}" )
      elseif ( ${_CMD} MATCHES PARAMS )
        set ( _PARAMS "${_PARAMS} ${_ARG}" )
      endif  ( ${_CMD} MATCHES PRE )
      set ( _CMD )
    endif ( ${ARG} MATCHES PRE )
  endforeach ( _ARG )
  
  if (_IGNOREOUT)
    set ( COMMAND "( ${PYTHON_EXECUTABLE} ${TESTNAME_LOC} ${_PARAMS} > /dev/null )" )
  else (_IGNOREOUT)
    set ( TESTNAME_OUT ${TESTNAME_TGT}.expout )
    set ( OUTFILE_LOC ${CMAKE_CURRENT_SOURCE_DIR}/${TESTNAME_OUT} ) 
    set ( COMMAND "( ${PYTHON_EXECUTABLE} ${TESTNAME_LOC} ${_PARAMS} > ${CMAKE_CURRENT_BINARY_DIR}/${TESTNAME_TGT}.out ) && diff -u --strip-trailing-cr ${OUTFILE_LOC} ${CMAKE_CURRENT_BINARY_DIR}/${TESTNAME_TGT}.out" )
  endif (_IGNOREOUT)

  if (_PRE OR _POST)
    set ( COMMAND "${_PRE}${COMMAND}${_POST}" )
  endif (_PRE OR _POST)

  add_test ( pyinstallcheck_${TESTNAME} "sh" "-c" "${COMMAND}" )

  set_tests_properties ( pyinstallcheck_${TESTNAME} PROPERTIES ENVIRONMENT "${PYINSTALLCHECK_ENVIRONMENT}" )
  list ( APPEND PYINSTALLCHECK_TO_BE_RUN ${TESTNAME_SRC} )
endmacro ( ot_pyinstallcheck_test )

ot_pyinstallcheck_test (MeasureEvaluation_std)
ot_pyinstallcheck_test (MeasureFactory_std)
ot_pyinstallcheck_test (MeasureFunction_std)
ot_pyinstallcheck_test (SequentialMonteCarloRobustAlgorithm_std)
ot_pyinstallcheck_test (docstring)
if (NOT CMAKE_SIZEOF_VOID_P STREQUAL "4") # cobyla/i686
  ot_pyinstallcheck_test (example1)
  ot_pyinstallcheck_test (example2)
  ot_pyinstallcheck_test (example3)
  ot_pyinstallcheck_test (example4)
endif ()
ot_pyinstallcheck_test (saveload)

add_custom_target ( pyinstallcheck COMMAND ${CMAKE_CTEST_COMMAND} -R "^pyinstallcheck_"
                    DEPENDS ${PYINSTALLCHECK_TO_BE_RUN}
                    COMMENT "Run Python post-installation tests" )


