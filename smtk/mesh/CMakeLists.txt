# set up sources to build
set(meshSrcs
  # Cells.cxx
  # CellSet.cxx
  Collection.cxx
  Manager.cxx
  # Mesh.cxx
  # MeshSet.cxx
  # Points.cxx
  # PointSet.cxx
  )

set(meshHeaders
  # Cells.h
  # CellSet.h
  CellTraits.h
  CellTypes.h
  DimensionTypes.h
  Manager.h
  Collection.h
  # Mesh.h
  # MeshSet.h
  # Points.h
  # PointSet.h
  QueryTypes.h
  moab/CellTypesToTypes.h
  )

#install the headers
smtk_public_headers(${meshHeaders})

if(SMTK_ENABLE_TESTING)
  add_subdirectory(testing)
endif()
