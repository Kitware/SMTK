# set up sources to build
set(meshSrcs
  # Cells.cxx
  # CellSet.cxx
  Collection.cxx
  Manager.cxx
  # Mesh.cxx
  MeshSet.cxx
  # Points.cxx
  # PointSet.cxx
  TypeSet.cxx
  moab/Interface.cxx
  )

set(meshHeaders
  # Cells.h
  # CellSet.h
  CellTraits.h
  CellTypes.h
  DimensionTypes.h
  Handle.h
  Manager.h
  Collection.h
  # Mesh.h
  MeshSet.h
  # Points.h
  # PointSet.h
  QueryTypes.h
  TypeSet.h
  moab/Interface.h
  )

#install the headers
smtk_public_headers(${meshHeaders})

if(SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()
