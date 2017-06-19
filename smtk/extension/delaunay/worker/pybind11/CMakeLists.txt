set(library_name "_smtkPybindMeshWorkerDelaunay")
set(module_path "mesh/delaunay/worker")
set(build_path "${PROJECT_BINARY_DIR}/smtk/${module_path}")
set(install_path "${SMTK_PYTHON_MODULEDIR}/smtk/${module_path}")

pybind11_add_module(${library_name} PybindMeshWorkerDelaunay.cxx)
target_include_directories(${library_name} PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
  )
target_link_libraries(${library_name} LINK_PUBLIC
  DelaunayShape
  DelaunayMesh
  DelaunayMisc
  DelaunayValidation
  DelaunayDiscretization
  DelaunayWorker
  RemusCommon
  RemusProto
  RemusWorker
  smtkCore
  smtkDelaunayExt
)
set_target_properties(${library_name}
  PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${build_path}"
    COMPILE_FLAGS ${SMTK_PYBIND11_FLAGS}
  )

get_filename_component(build_search_path "${CMAKE_BINARY_DIR}/bin" ABSOLUTE)
get_filename_component(install_search_path "${CMAKE_INSTALL_PREFIX}/bin" ABSOLUTE)
target_compile_definitions(${library_name} PRIVATE "BUILD_SEARCH_PATH=\"${build_search_path}\"")
target_compile_definitions(${library_name} PRIVATE "INSTALL_SEARCH_PATH=\"${install_search_path}\"")

# Install library
install(TARGETS ${library_name} DESTINATION "${install_path}")

# Create and install module __init__.py
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/__init__.py"
  "${build_path}/__init__.py" @ONLY
  )

install(
  FILES "${build_path}/__init__.py"
  DESTINATION "${install_path}"
  )
