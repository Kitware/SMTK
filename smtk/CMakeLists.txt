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
    $<INSTALL_INTERFACE:include/smtk/${SMTK_VERSION}>
)
target_link_libraries(smtkCore
  LINK_PUBLIC cJSON MOAB
  LINK_PRIVATE ${Boost_LIBRARIES})

target_include_directories(SMTKCore
  PRIVATE ${MOAB_INCLUDE_DIRS})

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

# On Mac OS X, set the dir. included as part of the installed library's path:
if (BUILD_SHARED_LIBS)
  set_target_properties(smtkCore
    PROPERTIES INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")
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
    SystemConfig.h
  DESTINATION
    include/smtk/${SMTK_VERSION}/smtk
)

################################################################################
# Generate Python wrapping of the smtkCore library

if(SMTK_ENABLE_PYTHON_WRAPPING AND Shiboken_FOUND)
  sbk_wrap_library(smtkCore
    GENERATOR_ARGS --avoid-protected-hack
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    LOCAL_INCLUDE_DIRECTORIES
      ${CMAKE_CURRENT_SOURCE_DIR}/common
      ${CMAKE_CURRENT_SOURCE_DIR}/attribute
      ${CMAKE_CURRENT_SOURCE_DIR}/model
      ${CMAKE_CURRENT_SOURCE_DIR}/simulation
      ${CMAKE_CURRENT_SOURCE_DIR}/io
      ${CMAKE_CURRENT_SOURCE_DIR}
      ${CMAKE_CURRENT_BINARY_DIR}
    TYPESYSTEM typesystem.xml
    HEADERS ${smtk_headers}
  )

  # Copy the shiboken library so developers have an
  # easier time testing the python wrapping.

  # First we have to resolve the path since shiboken library most
  # likely uses symlinks and we have to copy that version
  get_filename_component(shiboken_path "${SHIBOKEN_LIBRARY}" REALPATH)
  file(COPY "${shiboken_path}" DESTINATION "${SMTK_BINARY_DIR}")

  set(SMTK_PYTHON_MODULE_SRCS
    __init__
    simple
    testing
  )
  set(SMTK_PYTHON_MODULEFILES)
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

  # Store the location where we'll install SMTK's Python source
  # in the SMTK_PYTHON_MODULEFILES variable and add an install rule.
  # Note that SMTK_PYTHON_MODULEDIR may be provided if SMTK is being
  # built as a submodule or as part of a superbuild.
  if (NOT DEFINED SMTK_PYTHON_MODULEDIR)
    if (SMTK_INSTALL_PYTHON_TO_SITE_PACKAGES)
      execute_process(
        COMMAND
          ${PYTHON_EXECUTABLE}
          -c "import site; print(site.getsitepackages())[-1]"
        RESULT_VARIABLE SMTK_PYTHON_MODULEDIR
      )
    else()
      # NB. LIBEXECDIR is used because SMTK is a "non-pure" package (it depends
      # on compiled code rather than being implemented purely as python).
      # See https://docs.python.org/2/install/index.html for more.
      set(SMTK_PYTHON_MODULEDIR
        "${CMAKE_INSTALL_LIBEXECDIR}/python/site-packages")
    endif()
  endif()

  install(
    FILES ${SMTK_PYTHON_MODULEFILES}
    DESTINATION "${SMTK_PYTHON_MODULEDIR}/smtk"
  )
  install(CODE
    "set(LIBRARY_OUTPUT_PATH \"${CMAKE_INSTALL_PREFIX}/lib\")
     set(SHIBOKEN_LIBRARY \"${CMAKE_INSTALL_PREFIX}/lib/${SHIBOKEN_LIBRARY_NAME}\")
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
