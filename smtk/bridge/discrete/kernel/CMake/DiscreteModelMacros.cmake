# Create cmb-kit glue code for a server manager extension
# so that the kit can be used as a library for other plugins
# This macro will create both vtk{KIT} and vtk{KIT}CS libraries
MACRO(ADD_DISCRETE_MODEL_KIT KIT KIT_UPCASE Depend_LIBS Kit_SRCS)

  # Cache some CMake variables.
  SET(_VTK_BINARY_DIR "${VTK_BINARY_DIR}")
  SET(_VTK_KITS_DIR "${VTK_KITS_DIR}")

  SET(PV_INSTALL_NO_LIBRARIES 1)
  SET(VTK_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")
  SET(VTK_KITS_DIR "${CMAKE_CURRENT_BINARY_DIR}/Utilities")

  INCLUDE(${ParaView_CMAKE_DIR}/VTKMono/vtkExportKit.cmake)

  VTK_EXPORT_KIT("${KIT}" "${KIT_UPCASE}" "${Kit_SRCS}")

  # The VTK may be submodule of Paraview.
  #  IF(PARAVIEW_BUILD_SHARED_LIBS OR BUILD_SHARED_LIBS)
  #    SET(LIBTYPE SHARED)
  #  ELSE(PARAVIEW_BUILD_SHARED_LIBS OR BUILD_SHARED_LIBS)
  #    SET(LIBTYPE STATIC)
  #  ENDIF(PARAVIEW_BUILD_SHARED_LIBS OR BUILD_SHARED_LIBS)

  # Build as shared
  SET(LIBTYPE SHARED)

  # Create the library.
  PVVTK_ADD_LIBRARY(vtk${KIT} ${LIBTYPE}
    ${Kit_SRCS}
    )

  TARGET_LINK_LIBRARIES(vtk${KIT}
    ${Depend_LIBS}
    )

  # Reset some CMake variable
  SET(VTK_BINARY_DIR "${_VTK_BINARY_DIR}")
  SET(VTK_KITS_DIR "${_VTK_KITS_DIR}")

ENDMACRO(ADD_DISCRETE_MODEL_KIT)
