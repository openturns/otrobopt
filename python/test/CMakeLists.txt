

set (PATH_SEP ":")
if (WIN32)
  set (PATH_SEP "\\;")
endif ()


set ( PYINSTALLCHECK_ENVIRONMENT "OPENTURNS_CONFIG_PATH=${INSTALL_DESTDIR}${SYSCONFIG_PATH}/openturns"
                                 "PYTHONPATH=${OPENTURNS_PYTHON_MODULE_PATH}${PATH_SEP}${INSTALL_DESTDIR}${PYTHON_MODULE_PATH}${PATH_SEP}$ENV{PYTHONPATH}"
                                 "MPLBACKEND=agg"
    )


macro (ot_pyinstallcheck_test TESTNAME)
  set (TEST_TARGET t_${TESTNAME})
  set (TESTNAME_LOC ${CMAKE_CURRENT_SOURCE_DIR}/${TEST_TARGET}.py)

  set (_IGNOREOUT OFF)
  foreach (_ARG ${ARGN})
    if (${_ARG} MATCHES IGNOREOUT)
      set (_IGNOREOUT ON)
    endif ()
  endforeach ()

  if (_IGNOREOUT)
    add_test (NAME pyinstallcheck_${TESTNAME} COMMAND ${Python_EXECUTABLE} ${TESTNAME_LOC})
  else ()
    set (OUTFILE_LOC ${CMAKE_CURRENT_SOURCE_DIR}/${TEST_TARGET}.expout)
    if (NOT EXISTS ${OUTFILE_LOC})
      message (SEND_ERROR "Missing file ${OUTFILE_LOC}")
    endif ()
    add_test (NAME pyinstallcheck_${TESTNAME} COMMAND "sh" "-c" "(${Python_EXECUTABLE} ${TESTNAME_LOC} > ${CMAKE_CURRENT_BINARY_DIR}/${TEST_TARGET}.out) && diff -u --strip-trailing-cr ${OUTFILE_LOC} ${CMAKE_CURRENT_BINARY_DIR}/${TEST_TARGET}.out")
  endif ()

  set_tests_properties (pyinstallcheck_${TESTNAME} PROPERTIES ENVIRONMENT "${PYINSTALLCHECK_ENVIRONMENT}")
endmacro ()

ot_pyinstallcheck_test (MeasureEvaluation_std)
ot_pyinstallcheck_test (MeasureFactory_std)
ot_pyinstallcheck_test (MeasureFunction_std)
ot_pyinstallcheck_test (SequentialMonteCarloRobustAlgorithm_std)
ot_pyinstallcheck_test (SubsetInverseSampling_R-S)
ot_pyinstallcheck_test (SubsetInverseSampling_Waarts_system_series)
ot_pyinstallcheck_test (InverseFORM_std IGNOREOUT)
ot_pyinstallcheck_test (docstring)

if (MATPLOTLIB_FOUND)
  file (GLOB_RECURSE PYFILES "${PROJECT_SOURCE_DIR}/python/doc/examples/*.py")
  foreach (PYF ${PYFILES})
    if (PYF MATCHES plot)
      get_filename_component (TESTNAME "${PYF}" NAME_WE)
      add_test (NAME pyinstallcheck_example_${TESTNAME} COMMAND ${Python_EXECUTABLE} ${PYF})
      set_tests_properties (pyinstallcheck_example_${TESTNAME} PROPERTIES ENVIRONMENT "${PYINSTALLCHECK_ENVIRONMENT}")
    endif ()
  endforeach ()
endif ()

ot_pyinstallcheck_test (saveload)

add_custom_target ( pyinstallcheck COMMAND ${CMAKE_CTEST_COMMAND} -R "^pyinstallcheck_"
                    COMMENT "Run Python post-installation tests" )


