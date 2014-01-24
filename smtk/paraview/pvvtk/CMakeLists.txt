
# Although these are set in the toplevel CMakeLists, apparently
# the vtk module macros need to see them again:
include_directories(
  ${SMTK_SOURCE_DIR}
  ${SMTK_BINARY_DIR}
  ${SMTK_SOURCE_DIR}/ThirdParty
  ${SMTK_BINARY_DIR}/ThirdParty
  ${SMTK_SOURCE_DIR}/ThirdParty/cJSON
  ${Paraview_INCLUDE_DIRS}
)

set(pvSMTK_SRC
  vtkSMTKModelReader.cxx
)

set(pvSMTK_HEADERS
  vtkSMTKModelReader.h
)
# set_source_files_properties(
#   ${pvSMTK_Unwrapped_srcs}
#   WRAP_EXCLUDE
# )

set(pvSMTK_NO_HeaderTest 1)

vtk_module_library(pvSMTK ${pvSMTK_SRC})

#install the headers
smtk_public_headers(${pvSMTK_HEADERS})

target_link_libraries(pvSMTK LINK_PRIVATE SMTKCore)
smtk_install_library(pvSMTK)