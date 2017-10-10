################################################################################
# Each subdirectory exports variables naming header and source files
# to its parent scope (this CMakeLists.txt) which we combine into
# the smtkCore library.

add_subdirectory(common)
smtk_source_group(common)

add_subdirectory(resource)
smtk_source_group(resource)

add_subdirectory(attribute)
smtk_source_group(attribute)

add_subdirectory(operation)
smtk_source_group(operation)

add_subdirectory(mesh)
smtk_source_group(mesh)

add_subdirectory(model)
smtk_source_group(model)

add_subdirectory(simulation)
smtk_source_group(simulation)

add_subdirectory(io)
smtk_source_group(io)

#add_subdirectory(view)
#smtk_source_group(view)

set(smtk_headers
  ${commonHeaders}
  ${resourceHeaders}
  ${attributeHeaders}
  ${operationHeaders}
  ${meshHeaders}
  ${modelHeaders}
  ${simulationHeaders}
  ${ioHeaders}
)
set(smtk_srcs
  ${commonSrcs}
  ${resourceSrcs}
  ${attributeSrcs}
  ${operationSrcs}
  ${meshSrcs}
  ${modelSrcs}
  ${simulationSrcs}
  ${ioSrcs}
)

set(smtkCore_public_include_directories
  $<BUILD_INTERFACE:${SMTK_SOURCE_DIR}>
  $<BUILD_INTERFACE:${SMTK_BINARY_DIR}>
  $<INSTALL_INTERFACE:include/smtk/${SMTK_VERSION}>
  )

set(smtkCore_private_include_directories
  )

set(smtkCore_public_link_libraries
  cJSON
  MOAB
  )

set(smtkCore_private_link_libraries
  ${Boost_LIBRARIES}
  )

if(SMTK_ENABLE_PYTHON_WRAPPING)
  list(APPEND smtkCore_public_link_libraries
    pybind11::embed
    )
  add_subdirectory(pybind11)
endif()

add_library(smtkCore ${smtk_srcs})
target_include_directories(smtkCore
  PUBLIC
  ${smtkCore_public_include_directories}
  PRIVATE
  ${smtkCore_private_include_directories}
)
target_link_libraries(smtkCore
  LINK_PUBLIC ${smtkCore_public_link_libraries}
  LINK_PRIVATE ${smtkCore_private_link_libraries})

smtk_export_header(smtkCore CoreExports.h)
if (NOT BUILD_SHARED_LIBS AND SMTK_ENABLE_PYTHON_WRAPPING)
  # Set position-independent flag when other project libs are shared.
  set_target_properties(smtkCore PROPERTIES POSITION_INDEPENDENT_CODE True)
endif()

# On Mac OS X, link to the Foundation framework
if (APPLE)
  find_library(SMTK_MACOS_FOUNDATION_FMWK
    NAMES Foundation
    PATHS ${CMAKE_OSX_SYSROOT}/System/Library
    PATH_SUFFIXES Frameworks
    NO_DEFAULT_PATH)
  if (SMTK_MACOS_FOUNDATION_FMWK)
    target_link_libraries(smtkCore
      LINK_PRIVATE ${SMTK_MACOS_FOUNDATION_FMWK})
  endif()
  mark_as_advanced(SMTK_MACOS_FOUNDATION_FMWK)
endif()

#install the library and exports the library when used from a build tree
smtk_install_library(smtkCore)

################################################################################
# setup install rules
################################################################################
install(
  FILES
    AutoInit.h
    PublicPointerDefs.h
    SharedFromThis.h
    SharedPtr.h
    SystemConfig.h
  DESTINATION
    include/smtk/${SMTK_VERSION}/smtk
)

################################################################################
# Generate Python wrapping of the smtkCore library

if (SMTK_ENABLE_PYTHON_WRAPPING)

  #only install PythonAutoInit.h if pybind11 is enabled, since it directly
  #includes pybind11 headers
  install(
    FILES
      PythonAutoInit.h
    DESTINATION
      include/smtk/${SMTK_VERSION}/smtk
    )

  set(SMTK_PYTHON_MODULE_SRCS
    simple
    testing
    )
  foreach(pyfile ${SMTK_PYTHON_MODULE_SRCS})
    configure_file(
      "${CMAKE_CURRENT_SOURCE_DIR}/smtk/${pyfile}.py"
      "${SMTK_BINARY_DIR}/smtk/${pyfile}.py" @ONLY
      )
    list(APPEND SMTK_PYTHON_MODULEFILES "smtk/${pyfile}.py")
  endforeach()
    install(
	FILES ${SMTK_PYTHON_MODULEFILES}
	DESTINATION "${SMTK_PYTHON_MODULEDIR}/smtk"
	)
endif()

################################################################################
# Build smtk extensions
# Add libraries that extend other projects to have support for SMTK.
# For example extension/vtk allows you to show smtk models in VTK.
################################################################################
add_subdirectory(extension)

################################################################################
# Build model bridges
# Now that the main components of smtk have been built we can build
# any custom model bridge sessions that the user has enabled.
################################################################################
add_subdirectory(bridge)
