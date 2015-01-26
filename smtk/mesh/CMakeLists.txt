# set up sources to build
set(meshSrcs
  # Cells.cxx
  CellSet.cxx
  Collection.cxx
  Manager.cxx
  # Mesh.cxx
  MeshSet.cxx
  PointConnectivity.cxx
  # Points.cxx
  # PointSet.cxx
  TypeSet.cxx
  moab/CellTypeToType.cxx
  moab/Helpers.cxx
  moab/Interface.cxx
  moab/PointConnectivityStorage.cxx
  moab/Tags.cxx
  )

set(meshHeaders
  # Cells.h
  CellSet.h
  CellTraits.h
  CellTypes.h
  Collection.h
  DimensionTypes.h
  Handle.h
  Manager.h
  # Mesh.h
  MeshSet.h
  PointConnectivity.h
  # Points.h
  # PointSet.h
  QueryTypes.h
  TypeSet.h

  #only exposed moab headers are Interface, and Helpers. We need to work
  #on a better name. How about Interface and InterfaceIO?
  moab/Interface.h
  moab/Helpers.h
  )

#install the headers
smtk_public_headers(${meshHeaders})

if(SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()
