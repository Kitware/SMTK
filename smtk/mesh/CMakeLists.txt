# set up sources to build
set(meshSrcs
  CellSet.cxx
  CellTypes.cxx
  Collection.cxx
  Manager.cxx
  MeshSet.cxx
  PointConnectivity.cxx
  PointSet.cxx
  TypeSet.cxx
  moab/Allocator.cxx
  moab/CellTypeToType.cxx
  moab/Interface.cxx
  moab/ConnectivityStorage.cxx
  moab/Readers.cxx
  moab/Writers.cxx
  )

set(meshHeaders
  CellSet.h
  CellTraits.h
  CellTypes.h
  Collection.h
  DimensionTypes.h
  Handle.h
  Interface.h
  Manager.h
  MeshSet.h
  PointConnectivity.h
  PointSet.h
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
