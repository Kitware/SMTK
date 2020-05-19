# Classes have both a header and a source file:
set(classes
  IconConstructor
  Operation
  Session
  Registrar
  Resource
  Shape
)
# Operations have header, source, and sbt files:
set(operators
  CreateBox
  CreateResource
  Cut
  Import
)
# This should be initialized with a list of standalone header files:
set(headers
  Compound.h
  CompSolid.h
  Solid.h
  Shell.h
  Face.h
  Wire.h
  Edge.h
  Vertex.h
)
# This should be initialized with a list of standalone source files:
set(sources
)

# Expand class names into actual files
foreach(class ${classes})
  list(APPEND sources ${class}.cxx)
  list(APPEND headers ${class}.h)
endforeach()

# Expand operations into actual files plus a target for sbt encoding.
foreach (operator ${operators})
  smtk_encode_file("${CMAKE_CURRENT_SOURCE_DIR}/operators/${operator}.sbt"
  TARGET_OUTPUT targetName)
  list(APPEND sources operators/${operator}.cxx)
  list(APPEND headers operators/${operator}.h)
  list(APPEND _dependencies ${targetName})
endforeach()

# Build the library:
add_library(smtkOpencascadeSession ${sources})
add_dependencies(smtkOpencascadeSession ${_dependencies})
target_include_directories(smtkOpencascadeSession
  PUBLIC
    $<BUILD_INTERFACE:${OpenCASCADE_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:${OpenCASCADE_INCLUDE_DIR}>
)
target_link_libraries(smtkOpencascadeSession
  PUBLIC
    smtkCore
    TKGeomBase
    TKBRep
    TKernel
    TKLCAF
    TKXCAF
    TKSTEP
    TKXDESTEP
    TKIGES
    TKXDEIGES
)
smtk_export_header(smtkOpencascadeSession Exports.h)
smtk_install_library(smtkOpencascadeSession)
smtk_public_headers(smtkOpencascadeSession ${headers})

if (SMTK_ENABLE_VTK_SUPPORT)
  set(module_files "${CMAKE_CURRENT_SOURCE_DIR}/vtk/vtk.module")
  vtk_module_scan(
    MODULE_FILES ${module_files}
    PROVIDES_MODULES vtk_modules
    HIDE_MODULES_FROM_CACHE ON
    WANT_BY_DEFAULT ON)
  vtk_module_build(
    MODULES ${vtk_modules}
    PACKAGE SMTKOpencascadeExt
    INSTALL_EXPORT SMTKOpencascadeModules
    CMAKE_DESTINATION ${SMTK_INSTALL_CONFIG_DIR}
    HEADERS_DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/${PROJECT_VERSION}"
    TEST_DIRECTORY_NAME "NONE")

  if (SMTK_ENABLE_PARAVIEW_SUPPORT)
    set_property(GLOBAL APPEND
      PROPERTY _smtk_plugin_files "${CMAKE_CURRENT_SOURCE_DIR}/plugin/paraview.plugin")
  endif()
endif()

# if (SMTK_ENABLE_PYTHON_WRAPPING)
#   add_subdirectory(pybind11)
# endif()

# if (SMTK_ENABLE_TESTING)
#   add_subdirectory(testing)
# endif()
