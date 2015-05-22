
#Build the actual code
add_subdirectory(common)
smtk_source_group(common)

add_subdirectory(attribute)
smtk_source_group(attribute)

add_subdirectory(model)
smtk_source_group(model)

add_subdirectory(simulation)
smtk_source_group(simulation)

add_subdirectory(io)
smtk_source_group(io)

add_subdirectory(view)
smtk_source_group(view)

set(smtk_headers ${commonHeaders} ${attributeHeaders} ${modelHeaders} ${simulationHeaders} ${ioHeaders} ${viewHeaders})
set(smtk_srcs ${commonSrcs} ${attributeSrcs} ${modelSrcs} ${simulationSrcs} ${ioSrcs} ${viewSrcs})

add_library(smtkCore ${smtk_srcs})
target_link_libraries(smtkCore
  LINK_PUBLIC cJSON
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

# On Mac OS X, set the directory included as part of the installed library's path:
if (BUILD_SHARED_LIBS)
  set_target_properties(smtkCore PROPERTIES INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")
endif()

#install the library and exports the library when used from a build tree
smtk_install_library(smtkCore DEPENDS cJSON)

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

#setup the exports for the library when used from an installed location
install(EXPORT SMTK-targets DESTINATION lib)

#wrap everything
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
      ${CMAKE_CURRENT_SOURCE_DIR}/view
      ${CMAKE_CURRENT_SOURCE_DIR}
      ${CMAKE_CURRENT_BINARY_DIR}
    TYPESYSTEM typesystem.xml
    HEADERS ${smtk_headers}
  )

  #get the location to copy the shiboken library so developers have an
  #easier time testing the python wrapping

  #first we have to resolve the path since shiboken library most likely
  #uses symlinks and we have to copy that version
  get_filename_component(shiboken_path "${SHIBOKEN_LIBRARY}" REALPATH)
  file(COPY "${shiboken_path}" DESTINATION "${SMTK_BINARY_DIR}")

  set(SMTK_PYTHON_MODULE_SRCS
    __init__
    simple
    testing
  )
  foreach(pyfile ${SMTK_PYTHON_MODULE_SRCS})
    configure_file(
      "${CMAKE_CURRENT_SOURCE_DIR}/smtk/${pyfile}.py"
      "${SMTK_BINARY_DIR}/smtk/${pyfile}.py" @ONLY
    )
  endforeach()

  #todo we need to install this file in such a way that it can resolve
  #the location of shiboken, which is going to mean running fixup bundle
  #on the mac, and then fixing up the smtk.py package script

  get_filename_component(SHIBOKEN_LIBRARY_NAME ${SHIBOKEN_LIBRARY} NAME)
  install(CODE "set(LIBRARY_OUTPUT_PATH \"${CMAKE_INSTALL_PREFIX}/lib\")
                set(SHIBOKEN_LIBRARY \"${CMAKE_INSTALL_PREFIX}/lib/${SHIBOKEN_LIBRARY_NAME}\")
                configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/smtk/__init__.py ${CMAKE_INSTALL_PREFIX}/python/smtk/__init__.py )
                configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/smtk/testing.py ${CMAKE_INSTALL_PREFIX}/python/smtk/testing.py )
                configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/smtk/simple.py ${CMAKE_INSTALL_PREFIX}/python/smtk/simple.py )" )
endif()

################################################################################
# Build smtk extensions
#add libraries that extend other projects to have support for SMTK
#for example extension/vtk allows you to show smtk models in vtk
################################################################################
add_subdirectory(extension)

################################################################################
# Build model bridges
# Now that the main components of smtk have been built we can build the custom
# model bridge sessions that the user has enabled
################################################################################
add_subdirectory(bridge)
