find_package(PythonInterp REQUIRED)
find_package(PythonLibs REQUIRED)
find_package(Shiboken REQUIRED)
# find_package(PySide)

set(PYTHON_SHORT python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR})

# if(PySide_FOUND)
#   # Create 'virtual modules' for use as wrapping dependencies, starting with
#   # common properties (note that any PySide dependency requires using the
#   # pyside_global.h header, which in turn brings in all of Qt that is wrapped
#   # by PySide, hence every 'virtual module' needs include paths for everything)
#   set(SHIBOKEN_VIRTUAL_DEPENDENCIES)
#   foreach(_module Core Gui)
#     set(PySide:${_module}_TYPESYSTEM_PATHS ${PYSIDE_TYPESYSTEMS})
#     set(PySide:${_module}_WRAP_INCLUDE_DIRS
#       ${PYSIDE_INCLUDE_DIR}
#       ${PYSIDE_INCLUDE_DIR}/QtCore
#       ${PYSIDE_INCLUDE_DIR}/QtGui
#     )
#     set(PySide:${_module}_WRAP_LINK_LIBRARIES ${PYSIDE_LIBRARY})
#     set(PySide:${_module}_GLOBAL_HEADER pyside_global.h)
#     list(APPEND SHIBOKEN_VIRTUAL_DEPENDENCIES PySide:${_module})
#   endforeach()

#   # Set typesystem for each 'virtual module'
#   set(PySide:Core_TYPESYSTEM "typesystem_core.xml")
#   set(PySide:Gui_TYPESYSTEM "typesystem_gui.xml")
# endif()

set(SHIBOKEN_LIST_TYPESYSTEM_SOURCES_SCRIPT
  "${CMAKE_CURRENT_LIST_DIR}/sbk_list_typesystem_sources.py"
)

include(CMakeParseArguments)

###############################################################################

#------------------------------------------------------------------------------
# Function to concatenate strings in a list into a single string
function(sbk_cat VAR SEP)
  set(_result)
  foreach(_item ${ARGN})
    if(_result)
      set(_result "${_result}${SEP}${_item}")
    else()
      set(_result "${_item}")
    endif()
  endforeach()
  set(${VAR} "${_result}" PARENT_SCOPE)
endfunction()

#------------------------------------------------------------------------------
# Function to concatenate strings in a list into a single string, with
# duplicates suppressed
function(sbk_cat_no_dups VAR SEP)
  set(_result)
  if(ARGC GREATER 2)
    list(REMOVE_DUPLICATES ARGN)
    sbk_cat(_result "${SEP}" ${ARGN})
  endif()
  set(${VAR} "${_result}" PARENT_SCOPE)
endfunction()

#------------------------------------------------------------------------------
# Function to write content to a file, without spurious changes to time stamp
function(sbk_write_file PATH CONTENT)
  set(CMAKE_CONFIGURABLE_FILE_CONTENT "${CONTENT}")
  configure_file(
    "${CMAKE_ROOT}/Modules/CMakeConfigurableFile.in"
    "${PATH}" @ONLY
  )
endfunction()

#------------------------------------------------------------------------------
# Function to wrap a library using Shiboken
function(sbk_wrap_library NAME)
  set(_pyname ${NAME}Python)

  set(_single_value_args TYPESYSTEM WORKING_DIRECTORY)
  set(_named_arg_lists
    OBJECTS
    HEADERS
    DEPENDS
    EXTRA_INCLUDES
    LOCAL_INCLUDE_DIRECTORIES
    GENERATOR_ARGS
  )
  cmake_parse_arguments(""
    ""
    "${_single_value_args}"
    "${_named_arg_lists}"
    ${ARGN}
  )

  if(NOT _WORKING_DIRECTORY)
    set(_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  endif()

  # Get base include directories
  get_directory_property(_extra_include_dirs INCLUDE_DIRECTORIES)
  if(_LOCAL_INCLUDE_DIRECTORIES)
    list(APPEND _extra_include_dirs ${_LOCAL_INCLUDE_DIRECTORIES})
  else()
    list(APPEND _extra_include_dirs
      ${CMAKE_CURRENT_SOURCE_DIR}
      ${CMAKE_CURRENT_BINARY_DIR}
    )
  endif()

  # Get list of typesystem dependencies and build paths for the same
  set(_typesystem_depends)
  set(_typesystem_paths)
  set(_extra_typesystems)
  set(_extra_link_libraries)
  foreach(_dep ${_DEPENDS})
    if(NOT ${_dep}_TYPESYSTEM)
      message(SEND_ERROR "${NAME} dependency ${_dep} is not a wrapped library")
    else()
      # Get typesystem and typesystem paths for dependency
      if(IS_ABSOLUTE "${${_dep}_TYPESYSTEM}")
        list(APPEND _typesystem_depends "${${_dep}_TYPESYSTEM}")
      endif()
      get_filename_component(_dep_typesystem_name "${${_dep}_TYPESYSTEM}" NAME)
      get_filename_component(_dep_typesystem_path "${${_dep}_TYPESYSTEM}" PATH)
      list(APPEND _extra_typesystems
        "  <load-typesystem name=\"${_dep_typesystem_name}\" generate=\"no\"/>"
      )
      if(_dep_typesystem_path)
        list(APPEND _typesystem_paths "${_dep_typesystem_path}")
      endif()
      if(${_dep}_TYPESYSTEM_PATHS)
        list(APPEND _typesystem_paths "${${_dep}_TYPESYSTEM_PATHS}")
      endif()
      # Get global header for dependency
      if(${_dep}_GLOBAL_HEADER)
        list(APPEND _EXTRA_INCLUDES "${${_dep}_GLOBAL_HEADER}")
      endif()
      # Get include directories for dependency
      get_target_property(_target_includes ${_dep} INCLUDE_DIRECTORIES)
      list(APPEND _extra_include_dirs
        ${${_dep}_WRAP_INCLUDE_DIRS}
        ${_target_includes}
      )
      # Get additional link libraries for dependency (usually only set for
      # virtual modules)
      list(APPEND _extra_link_libraries ${${_dep}_WRAP_LINK_LIBRARIES})
    endif()
  endforeach()

  # Generate monolithic include file, as required by shiboken
  set(_global_header "${CMAKE_CURRENT_BINARY_DIR}/${NAME}_global.h")
  set(_depends)
  set(_content)
  foreach(_extra_include ${_EXTRA_INCLUDES})
    list(APPEND _content "#include <${_extra_include}>")
  endforeach()
  foreach(_hdr ${_HEADERS})
    get_filename_component(_hdr "${_hdr}" REALPATH)
    list(APPEND _depends "${_hdr}")
    list(APPEND _content "#include \"${_hdr}\"")
  endforeach()
  sbk_cat(_content "\n" ${_content})
  sbk_write_file("${_global_header}" "${_content}\n")

  # Get list of objects to wrap
  set(_objects)
  set(_type "object-type")
  foreach(_object ${_OBJECTS})
    if(_object STREQUAL "BY_REF")
      set(_type "object-type")
    elseif(_object STREQUAL "BY_VALUE")
      set(_type "value-type")
    elseif(_object STREQUAL "INTERFACES")
      set(_type "interface-type")
     else()
      list(APPEND _objects "  <${_type} name=\"${_object}\"/>")
    endif()
  endforeach()

  # Generate typesystem
  set(_typesystem "${CMAKE_CURRENT_BINARY_DIR}/${NAME}_typesystem.xml")
  if(_TYPESYSTEM)
    sbk_cat(EXTRA_TYPESYSTEMS "\n" "${_extra_typesystems}")
    sbk_cat(EXTRA_OBJECTS "\n" "${_objects}")
    set(TYPESYSTEM_NAME "${_pyname}")

    configure_file("${_TYPESYSTEM}" "${_typesystem}")
  else()
    sbk_cat(_content "\n"
      "<?xml version=\"1.0\"?>"
      "<typesystem package=\"${_pyname}\">"
      ${_extra_typesystems}
      ${_objects}
      "</typesystem>"
    )
    sbk_write_file("${_typesystem}" "${_content}\n")
  endif()

  # Determine list of generated source files
  execute_process(
    COMMAND "${PYTHON_EXECUTABLE}"
            "${SHIBOKEN_LIST_TYPESYSTEM_SOURCES_SCRIPT}"
            "${_typesystem}"
            "${WORKING_DIRECTORY}"
    WORKING_DIRECTORY "${_WORKING_DIRECTORY}"
    OUTPUT_VARIABLE _sources
  )
  string(REPLACE "\n" ";" _sources "${_sources}")
  if(NOT _sources)
    message(FATAL_ERROR "sbk_wrap_library: no generated source files found "
                        "for wrapped library ${NAME}")
  endif()
  set_source_files_properties(${_sources} PROPERTIES GENERATED TRUE)

  # Define rule to run the generator
  if(WIN32)
    sbk_cat_no_dups(_includes ";" ${_extra_include_dirs})
    sbk_cat_no_dups(_typesystem_paths ";" ${_typesystem_paths})
  else()
    sbk_cat_no_dups(_includes ":" ${_extra_include_dirs})
    sbk_cat_no_dups(_typesystem_paths ":" ${_typesystem_paths})
  endif()

  set(_shiboken_options
    --generatorSet=shiboken
    --enable-parent-ctor-heuristic
    --enable-return-value-heuristic
  )
  if(PySide_FOUND AND _DEPENDS MATCHES "^PySide:")
    list(APPEND _shiboken_options --enable-pyside-extensions)
  endif()
  if(_GENERATOR_ARGS)
    list(APPEND _shiboken_options ${_GENERATOR_ARGS})
  endif()

  add_custom_command(
    OUTPUT ${_sources}
    DEPENDS ${_typesystem} ${_global_header} ${_depends} ${_typesystem_depends}
    COMMAND "${SHIBOKEN_BINARY}"
            ${_shiboken_options}
            "${_global_header}"
            "--include-paths=${_includes}"
            "--typesystem-paths=${_typesystem_paths}"
            "--output-directory=${CMAKE_CURRENT_BINARY_DIR}"
            "${_typesystem}"
    WORKING_DIRECTORY ${_WORKING_DIRECTORY}
    COMMENT "Generating Python bindings for ${NAME}"
  )

  if(WIN32)
    set(_clean)
    FOREACH (i ${_sources})
      string(REGEX REPLACE "\\\\" "/" i ${i})
      set(_clean ${_clean} ${i})
    ENDFOREACH()
    set(_sources ${_clean})
  endif()

  # Remove "special" dependencies
  if(_DEPENDS)
    list(REMOVE_ITEM _DEPENDS ${SHIBOKEN_VIRTUAL_DEPENDENCIES})
  endif()

  # Declare the wrapper library
  add_library(${_pyname} MODULE ${_sources})
  set_property(TARGET ${_pyname} PROPERTY PREFIX "")
  set_property(TARGET ${_pyname} APPEND PROPERTY COMPILE_DEFINITIONS "SBK_WRAPPED_CODE")
  set_property(TARGET ${_pyname} APPEND PROPERTY INCLUDE_DIRECTORIES
               ${PYTHON_INCLUDE_DIRS}
               ${SHIBOKEN_INCLUDE_DIR}
               ${_extra_include_dirs}
              )
  if(WIN32)
    set_property(TARGET ${_pyname} PROPERTY SUFFIX ".pyd")
  endif()
  target_link_libraries(${_pyname} LINK_PRIVATE
    ${NAME}
    ${_DEPENDS}
    ${SHIBOKEN_PYTHON_LIBRARIES}
    ${SHIBOKEN_LIBRARY}
    ${_extra_link_libraries}
  )

  INSTALL(TARGETS ${_pyname} RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)

  foreach(_dep ${_DEPENDS})
    add_dependencies(${_pyname} ${_dep}Python)
  endforeach()

  # Record dependency information
  set(${NAME}_TYPESYSTEM
    "${_typesystem}"
    CACHE INTERNAL "Shiboken typesystem XML for ${NAME}"
  )
  set(${NAME}_GLOBAL_HEADER
    "${_global_header}"
    CACHE INTERNAL "Header file containing all includes for ${NAME}"
  )
  set(${NAME}_TYPESYSTEM_PATHS
    "${_typesystem_paths}"
    CACHE INTERNAL "Shiboken typesystem paths for ${NAME}"
  )
  set(${NAME}_WRAP_INCLUDE_DIRS
    "${CMAKE_CURRENT_BINARY_DIR}/${_pyname}"
    ${_extra_include_dirs}
    CACHE INTERNAL "Include directory for wrapped ${NAME} module"
  )
endfunction()
