
set ( ADAPT_CHECK_TESTS_EXPONENT )
set ( REPLACE_EOL " | cat " )
if ( MINGW )
  # replace dos eols
  set ( REPLACE_EOL " | tr -d '\\\\015' " )
  # By default, Mingw-g++ puts 3 digits to real exponent whereas g++ puts 2.
  set ( ADAPT_CHECK_TESTS_EXPONENT "PRINTF_EXPONENT_DIGITS=2" )
  
  add_library ( CRT_fp8 STATIC CRT_fp8.c )
  set_target_properties ( CRT_fp8 PROPERTIES LINKER_LANGUAGE CXX )
endif ()

set ( CHECK_ENVIRONMENT 
      ${ADAPT_CHECK_TESTS_EXPONENT} 
    )

set ( CHECK_TO_BE_RUN )
macro ( ot_check_test TESTNAME )
  set ( TESTNAME_TGT t_${TESTNAME} )
  set ( TESTNAME_SRC ${TESTNAME_TGT}.cxx )
  set ( TESTNAME_SRCS ${TESTNAME_SRC} )
  add_executable ( ${TESTNAME_TGT} EXCLUDE_FROM_ALL ${TESTNAME_SRCS} )
  add_dependencies ( tests ${TESTNAME_TGT} )
  target_link_libraries ( ${TESTNAME_TGT} otrobopt )
  if ( MINGW )
    target_link_libraries ( ${TESTNAME_TGT} CRT_fp8 )
  endif ()
  set_target_properties ( ${TESTNAME_TGT} PROPERTIES
                                          INSTALL_RPATH "${CMAKE_BINARY_DIR}/lib/src;${CMAKE_INSTALL_RPATH}" )
  set ( TESTNAME_LOC ${CMAKE_CURRENT_BINARY_DIR}/${TESTNAME_TGT}${CMAKE_EXECUTABLE_SUFFIX} ) 
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
    set ( COMMAND "( ${TESTNAME_LOC} ${_PARAMS} > /dev/null )" )
    set ( EXTERNAL_COMMAND "./${TESTNAME_TGT} ${_PARAMS} > $DEBUG_FILE 2>&1" )
  else (_IGNOREOUT)
    set ( TESTNAME_OUT ${TESTNAME_TGT}.expout )
    set ( OUTFILE_LOC ${CMAKE_CURRENT_SOURCE_DIR}/${TESTNAME_OUT} ) 
    set ( COMMAND "( ${TESTNAME_LOC} ${_PARAMS} ${REPLACE_EOL} > ${CMAKE_CURRENT_BINARY_DIR}/${TESTNAME_TGT}.out ) && diff -u ${OUTFILE_LOC} ${CMAKE_CURRENT_BINARY_DIR}/${TESTNAME_TGT}.out" )
    set ( EXTERNAL_COMMAND "( ./${TESTNAME_TGT} ${_PARAMS} 2>$DEBUG_FILE | tr -d '\\\\r' > ${TESTNAME_TGT}.out ) && diff -u ${TESTNAME_OUT} ${TESTNAME_TGT}.out" )
  endif (_IGNOREOUT)

  if (_PRE OR _POST)
    set ( COMMAND "${_PRE}${COMMAND}${_POST}" )
    set ( EXTERNAL_COMMAND "${_PRE}${EXTERNAL_COMMAND}${_POST}" )
  endif (_PRE OR _POST)

  add_test ( cppcheck_${TESTNAME} "sh" "-c" "${COMMAND}" )
  set_tests_properties ( cppcheck_${TESTNAME} PROPERTIES ENVIRONMENT "${CHECK_ENVIRONMENT}" )
  list ( APPEND CHECK_TO_BE_RUN ${TESTNAME_TGT} )
  install ( FILES ${TESTNAME_SRC}
            DESTINATION ${OTROBOPT_EXAMPLE_PATH} 
          )

  if ( INSTALL_TESTS )
    install ( FILES ${TESTNAME_OUT}
              DESTINATION ${OTROBOPT_EXAMPLE_PATH} 
            )
    install ( TARGETS ${TESTNAME_TGT}
              DESTINATION ${OTROBOPT_EXAMPLE_PATH} 
            )

#     add_check_file ( ${EXTERNAL_CHECK_FILE} ${TESTNAME_TGT} ${EXTERNAL_COMMAND} ${CUR_CHECK} )
#     math( EXPR CUR_CHECK "${CUR_CHECK} + 1" )
  endif ()

endmacro ( ot_check_test )

include_directories ( ${INTERNAL_INCLUDE_DIRS} )





ot_check_test ( MeasureFunction_std )


add_custom_target ( cppcheck COMMAND ${CMAKE_CTEST_COMMAND} -R "^cppcheck_"
                    DEPENDS ${CHECK_TO_BE_RUN}
                    COMMENT "Run C++ pre-installation tests" )
                    
                    


