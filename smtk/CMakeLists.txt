################################################################################
# Each subdirectory exports variables naming header and source files
# to its parent scope (this CMakeLists.txt) which we combine into
# the smtkCore library.

add_subdirectory(common)
smtk_source_group(common)

add_subdirectory(attribute)
smtk_source_group(attribute)

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
  ${attributeHeaders}
  ${meshHeaders}
  ${modelHeaders}
  ${simulationHeaders}
  ${ioHeaders}
)
set(smtk_srcs
  ${commonSrcs}
  ${attributeSrcs}
  ${meshSrcs}
  ${modelSrcs}
  ${simulationSrcs}
  ${ioSrcs}
)

add_library(smtkCore ${smtk_srcs})
target_include_directories(smtkCore
  PUBLIC
    $<BUILD_INTERFACE:${SMTK_SOURCE_DIR}>
    $<BUILD_INTERFACE:${SMTK_BINARY_DIR}>
    $<INSTALL_INTERFACE:include/smtk/${SMTK_VERSION}>
)
target_link_libraries(smtkCore
  LINK_PUBLIC cJSON MOAB
  LINK_PRIVATE ${Boost_LIBRARIES})

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

if (SMTK_USE_PYBIND11)
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
endif()

if(NOT SMTK_USE_PYBIND11 AND SMTK_ENABLE_PYTHON_WRAPPING)
  sbk_wrap_library(smtkCore
    PACKAGE smtk
    GENERATOR_ARGS --avoid-protected-hack
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    LOCAL_INCLUDE_DIRECTORIES
      ${CMAKE_CURRENT_SOURCE_DIR}/common
      ${CMAKE_CURRENT_SOURCE_DIR}/attribute
      ${CMAKE_CURRENT_SOURCE_DIR}/mesh
      ${CMAKE_CURRENT_SOURCE_DIR}/model
      ${CMAKE_CURRENT_SOURCE_DIR}/simulation
      ${CMAKE_CURRENT_SOURCE_DIR}/io
      ${CMAKE_CURRENT_SOURCE_DIR}
      ${CMAKE_CURRENT_BINARY_DIR}
	TYPESYSTEM typesystem.xml
	HEADERS ${smtk_headers}
	)

    set(SMTK_PYTHON_MODULE_SRCS
	__init__
	simple
	testing
	)
    set(SMTK_PYTHON_MODULEFILES)
    set(smtk_find_shiboken "build_tree")
    foreach(pyfile ${SMTK_PYTHON_MODULE_SRCS})
	configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/smtk/${pyfile}.py"
        "${SMTK_BINARY_DIR}/smtk/${pyfile}.py" @ONLY
	  )
	list(APPEND SMTK_PYTHON_MODULEFILES "smtk/${pyfile}.py")
    endforeach()

    # TODO: We need to install this file in such a way that it can resolve
    #       the location of shiboken, which is going to mean running fixup
    #       bundle on the mac, and then fixing up the smtk/__init__.py
    #       package script

    get_filename_component(SHIBOKEN_LIBRARY_NAME ${SHIBOKEN_LIBRARY} NAME)

    install(
	FILES ${SMTK_PYTHON_MODULEFILES}
	DESTINATION "${SMTK_PYTHON_MODULEDIR}/smtk"
	)
    set(SMTK_FIND_SHIBOKEN "install_tree"
	CACHE STRING "Set to empty if shiboken should be impicitly found in the install")
    mark_as_advanced(SMTK_FIND_SHIBOKEN)
    install(CODE
	"set(LIBRARY_OUTPUT_PATH \"${CMAKE_INSTALL_PREFIX}/lib\")
       set(SHIBOKEN_LIBRARY \"${CMAKE_INSTALL_PREFIX}/lib/${SHIBOKEN_LIBRARY_NAME}\")
       set(smtk_find_shiboken \"${SMTK_FIND_SHIBOKEN}\")
       configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/smtk/__init__.py
         ${CMAKE_INSTALL_PREFIX}/${SMTK_PYTHON_MODULEDIR}/smtk/__init__.py )"
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

if(SMTK_USE_PYBIND11)
  add_subdirectory(pybind11)
endif()
