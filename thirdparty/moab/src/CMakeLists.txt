include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    parallel
                    LocalDiscretization
                    io io/mhdf
                    ${MOAB_BINARY_DIR}/src/parallel )

add_subdirectory(verdict)
if(MOAB_HAVE_MPI AND MPI_FOUND)
  add_subdirectory(parallel)
  set( MOAB_MPI moabpar )
  set( MOABPAR_OBJECTS $<TARGET_OBJECTS:moabpar> )
  set( MPI_LIBRARIES ${MPI_LIBRARY} ${MPI_EXTRA_LIBRARY} )
endif()

#Create the moab/EntityHandle.hpp file
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/moab/EntityHandle.hpp.in infile2)
string( REGEX REPLACE "#undef" "#cmakedefine" outstring2 "${infile2}" )
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/moab/EntityHandle.hpp.in" "${outstring2}")
configure_file(${CMAKE_CURRENT_BINARY_DIR}/moab/EntityHandle.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/moab/EntityHandle.hpp)

set(moab_link_lib)
add_subdirectory(io)
add_subdirectory(RefineMesh)
add_subdirectory(LocalDiscretization)
# add_subdirectory(oldinc)

if( MOAB_HAVE_HDF5 )
  include_directories(${HDF5_INCLUDES})
  set( MOAB_HDF_LIBS ${HDF5_C_LIBRARIES} )
endif()
if( MOAB_HAVE_NETCDF )
  include_directories(${NETCDF_INCLUDES})
  if ( MOAB_HAVE_MPI )
    set( MOAB_NETCDF_LIBS ${PNETCDF_LIBRARIES} ${NETCDF_LIBRARIES} )
  else ( MOAB_HAVE_MPI )
    set( MOAB_NETCDF_LIBS ${NETCDF_LIBRARIES} )
  endif ( MOAB_HAVE_MPI )
endif()

set(MOAB_LIB_SOURCES
        AdaptiveKDTree.cpp
        AEntityFactory.hpp AEntityFactory.cpp
        AffineXform.hpp    AffineXform.cpp
        AxisBox.hpp        AxisBox.cpp
        BitPage.hpp        BitPage.cpp
        BitTag.hpp         BitTag.cpp
        BoundBox.cpp
        BSPTree.cpp
        BSPTreePoly.cpp
        BVHTree.cpp
        CN.cpp
        CartVect.cpp
        Core.cpp
        DebugOutput.hpp DebugOutput.cpp
        DenseTag.hpp    DenseTag.cpp
        DualTool.cpp
        ElementSequence.hpp
        EntitySequence.hpp  EntitySequence.cpp
        ErrorHandler.cpp
        ErrorOutput.hpp  ErrorOutput.cpp
        Factory.cpp
        FBEngine.cpp
        FileOptions.cpp
        GeomUtil.cpp
        GeomTopoTool.cpp
        HalfFacetRep.cpp
        HigherOrderFactory.cpp
        HomXform.cpp
        iMOAB.cpp
        Internals.hpp
        LloydSmoother.cpp
        MBCNArrays.hpp
        MergeMesh.cpp
        MeshSet.hpp         MeshSet.cpp
        MeshSetSequence.hpp MeshSetSequence.cpp
        MeshTag.hpp         MeshTag.cpp
        MeshTopoUtil.cpp
        OrientedBox.cpp
        OrientedBoxTreeTool.cpp
        lotte/poly.c
        lotte/findpt.c
        lotte/errmem.c
        lotte/tensor.c
        PolyElementSeq.hpp  PolyElementSeq.cpp
        ProgOptions.cpp
        Range.cpp
        RangeSeqIntersectIter.hpp RangeSeqIntersectIter.cpp
        ReadUtil.hpp              ReadUtil.cpp
        ReaderWriterSet.cpp
        ReorderTool.cpp
        ScdElementData.hpp        ScdElementData.cpp
        ScdInterface.cpp
        ScdVertexData.hpp         ScdVertexData.cpp
        SequenceData.hpp          SequenceData.cpp
        SequenceManager.hpp       SequenceManager.cpp
        SetIterator.cpp
        Skinner.cpp
        SmoothCurve.hpp           SmoothCurve.cpp
        SmoothFace.hpp            SmoothFace.cpp
        SparseTag.hpp             SparseTag.cpp
        SpatialLocator.cpp
        SpectralMeshTool.cpp
        StructuredElementSeq.hpp  StructuredElementSeq.cpp
        SweptElementData.hpp      SweptElementData.cpp
        SweptElementSeq.hpp       SweptElementSeq.cpp
        SweptVertexData.hpp       SweptVertexData.cpp
        SysUtil.hpp               SysUtil.cpp
        TagCompare.hpp
        TagInfo.hpp               TagInfo.cpp
        TupleList.cpp
        Tree.cpp
        Types.cpp
        TypeSequenceManager.hpp   TypeSequenceManager.cpp
        UnstructuredElemSeq.hpp   UnstructuredElemSeq.cpp
        Util.cpp
        VarLenDenseTag.hpp        VarLenDenseTag.cpp
        VarLenSparseTag.hpp       VarLenSparseTag.cpp
        VarLenTag.hpp
        VertexSequence.hpp        VertexSequence.cpp
        WriteUtil.hpp             WriteUtil.cpp
        moab_mpe.h )

set( MOAB_INSTALL_HEADERS
        moab/AdaptiveKDTree.hpp
        moab/BoundBox.hpp
        moab/BSPTree.hpp
        moab/BSPTreePoly.hpp
        moab/BVHTree.hpp
        moab/CN.hpp
        moab/CartVect.hpp
        moab/Compiler.hpp
        moab/Core.hpp
        moab/CpuTimer.hpp
        moab/DualTool.hpp
        moab/Error.hpp
        moab/GeomTopoTool.hpp
        moab/HalfFacetRep.hpp
        moab/HigherOrderFactory.hpp
        moab/HomXform.hpp
        moab/EntityType.hpp
        moab/ErrorHandler.hpp
        moab/FBEngine.hpp
        moab/FileOptions.hpp
        moab/FindPtFuncs.h
        moab/Forward.hpp
        moab/GeomUtil.hpp
        moab/iMOAB.h
        moab/Interface.hpp
        moab/LloydSmoother.hpp
        moab/point_locater/tree/common_tree.hpp
        moab/point_locater/tree/element_tree.hpp
        moab/point_locater/tree/bvh_tree.hpp
        moab/point_locater/io.hpp
        moab/point_locater/element_maps/linear_hex_map.hpp
        moab/point_locater/element_maps/linear_tet_map.hpp
        moab/point_locater/element_maps/spectral_hex_map.hpp
        moab/point_locater/element_maps/quadratic_hex_map.hpp
        moab/point_locater/point_locater.hpp
        moab/point_locater/parametrizer.hpp
        moab/Matrix3.hpp
        moab/MergeMesh.hpp
        moab/MeshTopoUtil.hpp
        moab/OrientedBox.hpp
        moab/OrientedBoxTreeTool.hpp
        moab/ProgOptions.hpp
        moab/Range.hpp
        moab/RangeMap.hpp
        moab/ReadUtilIface.hpp
        moab/ReaderIface.hpp
        moab/ReaderWriterSet.hpp
        moab/ReorderTool.hpp
        moab/ScdInterface.hpp
        moab/SetIterator.hpp
        moab/Skinner.hpp
        moab/SpatialLocator.hpp
        moab/SpectralMeshTool.hpp
        moab/Tree.hpp
        moab/TreeStats.hpp
        moab/TupleList.hpp
        moab/Types.hpp
        moab/mhdf_public.h
        moab/UnknownInterface.hpp
        moab/Util.hpp
        moab/WriteUtilIface.hpp
        moab/WriterIface.hpp
        MBEntityType.h
        MBCN.h
        MBCN_protos.h
        MBTagConventions.hpp )

set( MOAB_GENERATED_INSTALL_HEADERS
     ${CMAKE_CURRENT_BINARY_DIR}/moab/EntityHandle.hpp
     ${CMAKE_CURRENT_BINARY_DIR}/moab/MOABConfig.h
  )

if( MOAB_HAVE_CGM )
  set( MOAB_CGM_LIBS ${CGM_LIBRARIES} )
  include_directories(${CGM_INCLUDE_DIRS})
endif()

if (MOAB_HAVE_ZOLTAN)
  set( MOAB_LIB_SOURCES ${MOAB_LIB_SOURCES}
          ZoltanPartitioner.cpp )
  set( MOAB_INSTALL_HEADERS ${MOAB_INSTALL_HEADERS}
          moab/ZoltanPartitioner.hpp )
  include_directories( ${ZOLTAN_INCLUDES} )
  list(APPEND MOAB_DEP_LIBRARIES ${ZOLTAN_LIBRARIES})
endif()

if (MOAB_HAVE_METIS)
  set( MOAB_LIB_SOURCES ${MOAB_LIB_SOURCES}
        MetisPartitioner.cpp )
  set( MOAB_INSTALL_HEADERS ${MOAB_INSTALL_HEADERS}
        moab/MetisPartitioner.hpp )
  include_directories( ${METIS_INCLUDES} )
  list(APPEND MOAB_DEP_LIBRARIES ${METIS_LIBRARIES})
endif()

add_library(
  MOAB
  ${MOAB_LIB_SOURCES} ${MOAB_INSTALL_HEADERS} ${MOAB_GENERATED_INSTALL_HEADERS}
  $<TARGET_OBJECTS:moabio>
  $<TARGET_OBJECTS:moab_loc_discr>
  $<TARGET_OBJECTS:moab_refine_mesh>
  $<TARGET_OBJECTS:moabverdict>
  ${MOABPAR_OBJECTS}
  ${MDHF_OBJ}
  )

target_link_libraries(
  MOAB
  LINK_PUBLIC
  ${MOAB_DEP_LIBRARIES} ${MOAB_LIBS} ${MOAB_CGM_LIBS} ${MPI_LIBRARIES}
  )
set_target_properties(
  MOAB
  PROPERTIES
  COMPILE_FLAGS "-DIS_BUILDING_MB ${MOAB_DEFINES}"
  VERSION ${MOAB_VERSION}
  SOVERSION ${MOAB_VERSION_MAJOR}
  DEFINE_SYMBOL "IS_BUILDING_MB_SHARED"
  )
include(GenerateExportHeader)
generate_export_header(MOAB)
list(APPEND MOAB_GENERATED_INSTALL_HEADERS
  ${CMAKE_CURRENT_BINARY_DIR}/moab_export.h)

# On OS X, set the directory included as part of the installed library's path.
# We only do this to libraries we plan on installing.
if (BUILD_SHARED_LIBS)
  set_target_properties(MOAB
    PROPERTIES
    INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib"
    )
endif ()

target_include_directories(MOAB PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
  "$<INSTALL_INTERFACE:include>")

install(
  TARGETS MOAB
  EXPORT SMTK
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )
list(APPEND MOAB_INSTALL_TARGETS  MOAB)
include(AutoconfHeader)
moab_install_headers(${MOAB_INSTALL_HEADERS})
foreach (header ${MOAB_GENERATED_INSTALL_HEADERS})
    STRING(REGEX REPLACE "^${CMAKE_CURRENT_BINARY_DIR}/" "" relative ${header})
    string(REGEX MATCH "(.*)/" DIR ${relative})
    install(
      FILES ${header}
      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${DIR}
      )
endforeach()
