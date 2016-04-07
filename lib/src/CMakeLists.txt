

ot_add_current_dir_to_include_dirs ()

ot_add_source_file (MeasureFunction.cxx)
ot_add_source_file (MeasureFunctionImplementation.cxx)
ot_add_source_file (MeanMeasure.cxx)
ot_add_source_file (VarianceMeasure.cxx)
ot_add_source_file (MeanStandardDeviationTradeoffMeasure.cxx)
ot_add_source_file (QuantileMeasure.cxx)
ot_add_source_file (WorstCaseMeasure.cxx)
ot_add_source_file (AggregatedMeasure.cxx)
ot_add_source_file (MeasureFactory.cxx)
ot_add_source_file (RobustOptimizationProblem.cxx)
ot_add_source_file (RobustOptimizationAlgorithm.cxx)
ot_add_source_file (SequentialMonteCarloRobustAlgorithm.cxx)

ot_install_header_file (MeasureFunction.hxx)
ot_install_header_file (MeasureFunctionImplementation.hxx)
ot_install_header_file (MeanMeasure.hxx)
ot_install_header_file (VarianceMeasure.hxx)
ot_install_header_file (MeanStandardDeviationTradeoffMeasure.hxx)
ot_install_header_file (QuantileMeasure.hxx)
ot_install_header_file (WorstCaseMeasure.hxx)
ot_install_header_file (AggregatedMeasure.hxx)
ot_install_header_file (MeasureFactory.hxx)
ot_install_header_file (RobustOptimizationProblem.hxx)
ot_install_header_file (RobustOptimizationAlgorithm.hxx)
ot_install_header_file (SequentialMonteCarloRobustAlgorithm.hxx)


include_directories ( ${INTERNAL_INCLUDE_DIRS} )

add_library ( otrobopt ${SOURCEFILES} )
if ( BUILD_SHARED_LIBS )
  set_target_properties ( otrobopt PROPERTIES COMPILE_DEFINITIONS "OTROBOPT_DLL_EXPORTS" )
endif ()

if ( NOT DEFINED LIB_VERSION ) 
  set ( LIB_VERSION 0.0.0 )
endif ()
if ( NOT DEFINED LIB_SOVERSION ) 
  set ( LIB_SOVERSION 0 )
endif ()
set_target_properties ( otrobopt PROPERTIES VERSION ${LIB_VERSION} )
set_target_properties ( otrobopt PROPERTIES SOVERSION ${LIB_SOVERSION} )
target_link_libraries (otrobopt ${OPENTURNS_LIBRARY})

# Add targets to the build-tree export set
export (TARGETS otrobopt FILE ${PROJECT_BINARY_DIR}/OTRobOpt-Targets.cmake)

# Install the export set for use with the install-tree
install(EXPORT OTRobOpt-Targets
        DESTINATION "${OTROBOPT_CONFIG_CMAKE_PATH}"
        COMPONENT Development)

install(TARGETS otrobopt
        EXPORT  OTRobOpt-Targets 
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION ${OTROBOPT_LIBRARY_PATH}
        ARCHIVE DESTINATION ${OTROBOPT_LIBRARY_PATH}
)

