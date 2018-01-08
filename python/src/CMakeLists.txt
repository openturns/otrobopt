
include ( ${SWIG_USE_FILE} )

include_directories ( ${OPENTURNS_SWIG_INCLUDE_DIRS} )
add_definitions( ${OPENTURNS_SWIG_DEFINITIONS} )
include_directories ( ${INTERNAL_INCLUDE_DIRS} )
include_directories ( ${PYTHON_INCLUDE_DIRS} )

add_custom_target( generate_docstrings )

macro ( ot_add_python_module MODULENAME SOURCEFILE )
  set_source_files_properties ( ${SOURCEFILE} PROPERTIES CPLUSPLUS ON )
  set_source_files_properties ( ${SOURCEFILE} PROPERTIES SWIG_MODULE_NAME ${MODULENAME} )
  ot_install_swig_file ( ${SOURCEFILE} )
  if ( ${ARGC} GREATER 2 )
    set ( SWIG_MODULE_${MODULENAME}_EXTRA_DEPS ${ARGN} )
  endif ()

  set ( swig_other_sources )
  foreach ( it ${ARGN} )
    if ( ${it} MATCHES ".*\\.i$" )
      ot_install_swig_file ( ${it} )
    elseif(${it} MATCHES ".*_doc\\.i\\.in$")
      # handle docstring generation
      set (DOCSTRING_IN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/${it})
      get_filename_component ( DOCSTRING_FILE_BASENAME ${DOCSTRING_IN_FILE} NAME_WE )
      add_custom_target ( generate_${DOCSTRING_FILE_BASENAME}
                          COMMAND ${CMAKE_COMMAND} -DDOCSTRING_IN_FILE=${DOCSTRING_IN_FILE} -DDOCSTRING_FILE=${CMAKE_CURRENT_BINARY_DIR}/${DOCSTRING_FILE_BASENAME}.i -P ${CMAKE_SOURCE_DIR}/cmake/escape_backslash_x4.cmake
                          DEPENDS ${DOCSTRING_IN_FILE} )
      add_dependencies ( generate_docstrings generate_${DOCSTRING_FILE_BASENAME} )
      list ( APPEND SWIGFILES ${CMAKE_CURRENT_BINARY_DIR}/${DOCSTRING_FILE_BASENAME}.i )
    else ()
      set ( swig_other_sources ${swig_other_sources} "${it}" )
    endif ()
  endforeach ()

  # swig_add_module is deprecated
  if (CMAKE_VERSION VERSION_LESS 3.8)
    swig_add_module (${MODULENAME} python ${SOURCEFILE} ${swig_other_sources} )
  else ()
    swig_add_library (${MODULENAME} LANGUAGE python SOURCES ${SOURCEFILE} ${swig_other_sources})
  endif ()

  add_dependencies ( ${SWIG_MODULE_${MODULENAME}_REAL_NAME} generate_docstrings )
  swig_link_libraries ( ${MODULENAME} otrobopt )
  if ( LINK_PYTHON_LIBRARY )
    swig_link_libraries ( ${MODULENAME} ${PYTHON_LIBRARIES} )
  endif ()

  set_target_properties ( ${SWIG_MODULE_${MODULENAME}_REAL_NAME} PROPERTIES NO_SONAME ON )

  list ( APPEND PYTHON_MODULES ${MODULENAME} )
endmacro ( ot_add_python_module )



ot_add_python_module( ${PACKAGE_NAME} ${PACKAGE_NAME}_module.i 
                      MeasureEvaluation.i MeasureEvaluation_doc.i.in
                      MeasureEvaluationImplementation.i MeasureEvaluationImplementation_doc.i.in
                      MeanMeasure.i MeanMeasure_doc.i.in
                      VarianceMeasure.i VarianceMeasure_doc.i.in
                      MeanStandardDeviationTradeoffMeasure.i MeanStandardDeviationTradeoffMeasure_doc.i.in
                      QuantileMeasure.i QuantileMeasure_doc.i.in
                      WorstCaseMeasure.i WorstCaseMeasure_doc.i.in
                      AggregatedMeasure.i AggregatedMeasure_doc.i.in
                      JointChanceMeasure.i JointChanceMeasure_doc.i.in
                      IndividualChanceMeasure.i IndividualChanceMeasure_doc.i.in
                      MeasureFunction.i MeasureFunction_doc.i.in
                      MeasureFactory.i MeasureFactory_doc.i.in
                      RobustOptimizationProblem.i RobustOptimizationProblem_doc.i.in
                      RobustOptimizationAlgorithm.i RobustOptimizationAlgorithm_doc.i.in
                      SequentialMonteCarloRobustAlgorithm.i SequentialMonteCarloRobustAlgorithm_doc.i.in
                      SubsetInverseSamplingResult.i SubsetInverseSamplingResult_doc.i.in
                      SubsetInverseSampling.i SubsetInverseSampling_doc.i.in
                    )


foreach ( module ${PYTHON_MODULES} )
  install ( FILES ${CMAKE_CURRENT_BINARY_DIR}/${module}.py
            DESTINATION ${OTROBOPT_PYTHON_MODULE_PATH}/${PACKAGE_NAME}
            )
  install ( TARGETS _${module}
            LIBRARY DESTINATION ${OTROBOPT_PYTHON_MODULE_PATH}/${PACKAGE_NAME}
          )
endforeach ( module )

install ( FILES __init__.py
          DESTINATION ${OTROBOPT_PYTHON_MODULE_PATH}/${PACKAGE_NAME}
        )

install ( FILES ${SWIGFILES}
          DESTINATION ${OTROBOPT_SWIG_INCLUDE_PATH}
        )




if (SPHINX_FOUND AND SPHINX_EXECUTABLE)

  # create a build site-package from which sphinx can import
  set (OTROBOPT_PYTHON_BUILD_MODULE_PATH ${CMAKE_CURRENT_BINARY_DIR}/site-packages)
  execute_process (COMMAND ${CMAKE_COMMAND} -E make_directory ${OTROBOPT_PYTHON_BUILD_MODULE_PATH}/otrobopt)
  foreach (module ${PYTHON_MODULES})
    execute_process (COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_BINARY_DIR}/${module}.py ${OTROBOPT_PYTHON_BUILD_MODULE_PATH}/otrobopt/${module}.py)
    # FIXME: get_target_property (suffix ${SWIG_MODULE_${module}_REAL_NAME} SUFFIX)
    execute_process (COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_BINARY_DIR}/_${module}.so ${OTROBOPT_PYTHON_BUILD_MODULE_PATH}/otrobopt/_${module}.so)
  endforeach ()
  foreach (file __init__.py)
    execute_process (COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/${file} ${OTROBOPT_PYTHON_BUILD_MODULE_PATH}/otrobopt/${file})
  endforeach ()

  # sphinx configuration
  set (doc_formats)
  list (APPEND doc_formats html)
  list (APPEND doc_formats htmlhelp)
  list (APPEND doc_formats latex)
  list (APPEND doc_formats doctest)
  list (APPEND doc_formats qthelp)
  list (APPEND doc_formats devhelp)

  set (SPHINX_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/sphinx_build)
  set (SPHINX_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../doc)

  file (MAKE_DIRECTORY ${SPHINX_BINARY_DIR}/_static) # currently empty

  if (NOT DEFINED OPENTURNS_PYTHON_MODULE_PATH)
    set(OPENTURNS_PYTHON_MODULE_PATH ${OPENTURNS_PYTHON${PYTHON_VERSION_MAJOR}_MODULE_PATH})
  endif ()
  configure_file (${SPHINX_SOURCE_DIR}/conf.py.in ${SPHINX_BINARY_DIR}/conf.py @ONLY)

  # get sphinx sources
  file (GLOB_RECURSE sphinx_sources
    ${SPHINX_SOURCE_DIR}/*.rst
    ${SPHINX_SOURCE_DIR}/*.rst_t
    ${SPHINX_SOURCE_DIR}/*.css_t
    ${SPHINX_SOURCE_DIR}/*.conf
    ${SPHINX_SOURCE_DIR}/*.py
    ${SPHINX_SOURCE_DIR}/*.png
    ${SPHINX_SOURCE_DIR}/*.html
    ${SPHINX_SOURCE_DIR}/*.css
    ${SPHINX_SOURCE_DIR}/*.jpg
    ${SPHINX_SOURCE_DIR}/*.ico
    ${SPHINX_SOURCE_DIR}/*.sty
    ${SPHINX_SOURCE_DIR}/*.txt
    ${SPHINX_SOURCE_DIR}/*.in
  )

  # copy sphinx sources
  set (sphinx_clones)
  foreach (file ${sphinx_sources})
    if (EXISTS ${file})
      file ( RELATIVE_PATH rel_file ${SPHINX_SOURCE_DIR} ${file} )
      get_filename_component (rel_path ${rel_file} PATH)
      file (MAKE_DIRECTORY ${SPHINX_BINARY_DIR}/${rel_path})
      set (sphinx_clone ${SPHINX_BINARY_DIR}/${rel_file})
      add_custom_command (OUTPUT ${sphinx_clone}
        COMMAND ${CMAKE_COMMAND} -E copy ${file} ${sphinx_clone}
        DEPENDS ${file}
      )
      list (APPEND sphinx_clones ${sphinx_clone})
    endif ()
  endforeach ()

  # copy python tests in root
  file (GLOB_RECURSE python_tests
    ${CMAKE_SOURCE_DIR}/python/test/t_example*.py
  )
  foreach (file ${python_tests})
    if (EXISTS ${file})
      get_filename_component (name ${file} NAME)
      set (sphinx_clone ${SPHINX_BINARY_DIR}/${name})
      add_custom_command (OUTPUT ${sphinx_clone}
        COMMAND ${CMAKE_COMMAND} -E copy ${file} ${sphinx_clone}
        DEPENDS ${file}
      )
      list (APPEND sphinx_clones ${sphinx_clone})
    endif ()
  endforeach()
  
  set (SPHINX_FLAGS "-W" CACHE STRING "sphinx flags")
  separate_arguments (SPHINX_FLAGS)

  # generate sphinx rule for each format
  foreach (format ${doc_formats})
    add_custom_target ( sphinx_${format}
                        COMMAND ${SPHINX_EXECUTABLE}
                                  -b ${format}
                                  ${SPHINX_FLAGS}
                                  ${SPHINX_BINARY_DIR}
                                  ${SPHINX_BINARY_DIR}/${format}
                        DEPENDS ${sphinx_clones}
                      )
    foreach (module ${PYTHON_MODULES})
      add_dependencies (sphinx_${format} _${module})
    endforeach ()
  endforeach ()

  # build html by default
  set_target_properties (sphinx_html PROPERTIES EXCLUDE_FROM_ALL OFF)

  add_custom_command (OUTPUT ${SPHINX_BINARY_DIR}/latex/OpenTURNS.pdf
                      WORKING_DIRECTORY ${SPHINX_BINARY_DIR}/latex
                      COMMAND make all-pdf
                    )
  add_custom_target (sphinx_pdf DEPENDS ${SPHINX_BINARY_DIR}/latex/OpenTURNS.pdf)
  add_dependencies (sphinx_pdf sphinx_latex)

  install (DIRECTORY ${SPHINX_BINARY_DIR}/html DESTINATION ${OTROBOPT_DOC_PATH})
endif ()
