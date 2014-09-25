
#Build the actual code
add_subdirectory(attribute)
smtk_source_group(attribute)

add_subdirectory(model)
smtk_source_group(model)

add_subdirectory(util)
smtk_source_group(util)

add_subdirectory(view)
smtk_source_group(view)

set(smtk_headers ${attributeHeaders} ${modelHeaders} ${utilHeaders} ${viewHeaders})
set(smtk_srcs ${attributeSrcs} ${modelSrcs} ${utilSrcs} ${viewSrcs})

add_library(SMTKCore ${smtk_srcs})
target_link_libraries(SMTKCore
  LINK_PUBLIC cJSON
  LINK_PRIVATE ${Boost_LIBRARIES})

smtk_export_header(SMTKCore SMTKCoreExports.h)
if (NOT BUILD_SHARED_LIBS AND SMTK_BUILD_PYTHON_WRAPPINGS)
  # Set position-independent flag when other project libs are shared.
  set_target_properties(SMTKCore PROPERTIES POSITION_INDEPENDENT_CODE True)
endif()

# On Mac OS X, set the directory included as part of the installed library's path:
if (BUILD_SHARED_LIBS)
  set_target_properties(SMTKCore PROPERTIES INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")
endif()

#install the library and exports the library when used from a build tree
smtk_install_library(SMTKCore DEPENDS cJSON)

################################################################################
# setup install rules
################################################################################
install(FILES PublicPointerDefs.h DESTINATION include/smtk)

#setup the exports for the library when used from an installed location
install(EXPORT SMTK-targets DESTINATION lib)

#wrap everything
if(SMTK_BUILD_PYTHON_WRAPPINGS AND Shiboken_FOUND)
  sbk_wrap_library(SMTKCore
    GENERATOR_ARGS --avoid-protected-hack
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    LOCAL_INCLUDE_DIRECTORIES
      ${CMAKE_CURRENT_SOURCE_DIR}/attribute
      ${CMAKE_CURRENT_SOURCE_DIR}/model
      ${CMAKE_CURRENT_SOURCE_DIR}/util
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

  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/smtk.py"
                 "${SMTK_BINARY_DIR}/smtk.py" @ONLY)

  if(SMTK_ENABLE_TESTING)
    get_filename_component(SHIBOKEN_LIBRARY_PATH ${SHIBOKEN_LIBRARY} PATH)
    add_subdirectory(model/PythonTesting)
    add_subdirectory(view/PythonTesting)
  endif(SMTK_ENABLE_TESTING)

  #todo we need to install this file in such a way that it can resolve
  #the location of shiboken, which is going to mean running fixup bundle
  #on the mac, and than fixing up the smtk.py package script

  get_filename_component(SHIBOKEN_LIBRARY_NAME ${SHIBOKEN_LIBRARY} NAME)
  install(CODE "set(LIBRARY_OUTPUT_PATH \"${CMAKE_INSTALL_PREFIX}/lib\")
                set(SHIBOKEN_LIBRARY \"${CMAKE_INSTALL_PREFIX}/lib/${SHIBOKEN_LIBRARY_NAME}\")
                configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/smtk.py ${CMAKE_INSTALL_PREFIX}/python/smtk.py )" )
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
# model bridges that the user has enabled
################################################################################
add_subdirectory(bridge)