include_directories( ${MOAB_BINARY_DIR}/src
                     ${MOAB_SOURCE_DIR}/src
                     ${MOAB_SOURCE_DIR}/src/io
                     ${MOAB_SOURCE_DIR}/src/mhdf
                     ${MOAB_BINARY_DIR}/src/parallel
                    )
set( MOABIO_LIBS )
set( MOABIO_DEFINES "-DIS_BUILDING_MB" )
set( MOABIO_INSTALL_HEADERS moab/ExoIIInterface.hpp )
set( MOABIO_LIB_SRCS
        exodus_order.h
        patran_order.h
        IODebugTrack.hpp  IODebugTrack.cpp
        ExoIIUtil.hpp     ExoIIUtil.cpp
        FileTokenizer.hpp FileTokenizer.cpp
        GmshUtil.hpp      GmshUtil.cpp
        ReadABAQUS.hpp    ReadABAQUS.cpp
        ReadGmsh.hpp      ReadGmsh.cpp
        ReadIDEAS.hpp     ReadIDEAS.cpp
        ReadMCNP5.hpp     ReadMCNP5.cpp
        ReadNASTRAN.hpp   ReadNASTRAN.cpp
        ReadSmf.hpp       ReadSmf.cpp
        ReadSms.hpp       ReadSms.cpp
        ReadSTL.hpp       ReadSTL.cpp
        ReadTemplate.hpp  ReadTemplate.cpp
        ReadTetGen.hpp    ReadTetGen.cpp
        ReadTxt.hpp       ReadTxt.cpp
        ReadVtk.hpp       ReadVtk.cpp
        SMF_State.hpp     SMF_State.cpp
        Tqdcfr.hpp        Tqdcfr.cpp
        VtkUtil.hpp       VtkUtil.cpp
        WriteAns.hpp      WriteAns.cpp
        WriteGMV.hpp      WriteGMV.cpp
        WriteGmsh.hpp     WriteGmsh.cpp
        WriteSTL.hpp      WriteSTL.cpp
        WriteSmf.hpp      WriteSmf.cpp
        WriteTemplate.hpp WriteTemplate.cpp
        WriteVtk.hpp      WriteVtk.cpp )

if( MOAB_HAVE_NETCDF )
  set( MOAB_NETCDF_SRCS
          ReadNCDF.hpp      ReadNCDF.cpp
          WriteNCDF.hpp     WriteNCDF.cpp
          WriteSLAC.hpp     WriteSLAC.cpp
          ReadNC.hpp        ReadNC.cpp
          WriteNC.hpp       WriteNC.cpp
          NCHelper.hpp      NCHelper.cpp
          NCWriteHelper.hpp NCWriteHelper.cpp
          NCHelperEuler.hpp NCHelperEuler.cpp
          NCWriteEuler.hpp  NCWriteEuler.cpp
          NCHelperFV.hpp    NCHelperFV.cpp
          NCWriteFV.hpp     NCWriteFV.cpp
          NCHelperGCRM.hpp  NCHelperGCRM.cpp
          NCWriteGCRM.hpp   NCWriteGCRM.cpp
          NCHelperHOMME.hpp NCHelperHOMME.cpp
          NCWriteHOMME.hpp  NCWriteHOMME.cpp
          NCHelperMPAS.hpp  NCHelperMPAS.cpp
          NCWriteMPAS.hpp   NCWriteMPAS.cpp
       )
        set( MOABIO_LIBS ${MOABIO_LIBS} ${NETCDF_LIBRARIES} )
        include_directories(${NETCDF_INCLUDES})
  # check for PNetCDF dependencies also
  if( MOAB_HAVE_PNETCDF )
    set( MOABIO_LIBS ${MOABIO_LIBS} ${PNETCDF_LIBRARIES} )
    include_directories(${PNETCDF_INCLUDES})
  endif( MOAB_HAVE_PNETCDF )
endif( MOAB_HAVE_NETCDF )

if( MOAB_HAVE_ZOLTAN )
  include_directories(${ZOLTAN_INCLUDES})
endif (MOAB_HAVE_ZOLTAN)
 
if ( MOAB_HAVE_CGNS )
  set( MOAB_CGNS_SRCS ReadCGNS.hpp ReadCGNS.cpp
                      WriteCGNS.hpp WriteCGNS.cpp)
endif()

if( MOAB_HAVE_HDF5 )
  add_subdirectory( mhdf )
  set( MOAB_HDF5_SRCS
          HDF5Common.hpp HDF5Common.cpp
          ReadHDF5.hpp ReadHDF5.cpp
          ReadHDF5Dataset.hpp ReadHDF5Dataset.cpp
          ReadHDF5VarLen.hpp ReadHDF5VarLen.cpp
          WriteHDF5.cpp WriteHDF5.hpp )
  set( MDHF_OBJ $<TARGET_OBJECTS:mhdf> PARENT_SCOPE )
  set( MOABIO_LIBS ${MOABIO_LIBS} ${HDF5_LIBRARIES} )
  include_directories(${HDF5_INCLUDES})
  check_function_exists( H5Pset_fapl_mpio MOAB_HDF_HAVE_PARALLEL )
endif()

if( MOAB_HAVE_CCMIO )
  set( MOAB_CCMIO_SRCS WriteCCMIO.hpp WriteCCMIO.cpp 
                       ReadCCMIO.hpp ReadCCMIO.cpp )
endif()

if( MOAD_HAVE_DAMSEL )
  set( MOAB_DAMSEL_SRCS
          WriteDamsel.hpp WriteDamsel.cpp 
          ReadDamsel.hpp  ReadDamsel.cpp
          DamselUtil.hpp  DamselUtil.cpp )
endif()

if( MOAB_HAVE_CGM )
  set( MOAB_CGM_SRCS ReadCGM.hpp ReadCGM.cpp )
  list(APPEND MOABIO_LIBS cubit_geom_compat)
  include_directories(${CGM_INCLUDE_DIRS})
endif()

add_library( moabio OBJECT ${MOABIO_LIB_SRCS} ${MOAB_NETCDF_SRCS}
                           ${MOAB_CGNS_SRCS}  ${MOAB_HDF5_SRCS}
                           ${MOAB_CCMIO_SRCS} ${MOAB_DAMSEL_SRCS}
                           ${MOAB_CGM_SRCS}   ${MOABIO_INSTALL_HEADERS} )
set(MOABIO_LIBS "${MOABIO_LIBS}" PARENT_SCOPE)
if (BUILD_SHARED_LIBS)
  set( shared_define "-DIS_BUILDING_MB_SHARED" )
endif ()
set_target_properties( moabio PROPERTIES COMPILE_FLAGS "${shared_define} ${MOABIO_DEFINES} ${MOAB_DEFINES}")
include(AutoconfHeader)
moab_install_headers(${MOABIO_INSTALL_HEADERS})
