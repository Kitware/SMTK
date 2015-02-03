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
  moab/Allocator.cxx
  moab/CellTypeToType.cxx
  moab/Interface.cxx
  moab/PointConnectivityStorage.cxx
  moab/Readers.cxx
  moab/Writers.cxx
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

  #The only exposed backend headers are Interface currently
  moab/Interface.h
  moab/Allocator.h
  )

#install the headers
smtk_public_headers(${meshHeaders})

if(SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()
