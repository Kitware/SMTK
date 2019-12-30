# The configured header file used to pass CMake settings to the source code.
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/Model/VTKUUIDConfigure.h.in"
  "${PROJECT_BINARY_DIR}/VTKUUIDConfigure.h"
  )


###############################################################################
###############################################################################
# UUID stuff. Probably don't need all of it
set(UUID_LIBS)
if(WIN32)
 if (NOT BORLAND)
    # rpcrt4 -> UuidCreate
    #snmpapi -> getting MAC address
    set(UUID_LIBS ${KIT_LIBS} rpcrt4 snmpapi)
  endif(NOT BORLAND)
endif(WIN32)

# Check if header file exists and add it to the list.
include (${CMAKE_ROOT}/Modules/CheckIncludeFiles.cmake)
MACRO(CHECK_INCLUDE_FILE_CONCAT FILE VARIABLE)
  CHECK_INCLUDE_FILES("${KWECommon_INCLUDES};${FILE}" ${VARIABLE})
  if(${VARIABLE})
    set(KWECommon_INCLUDES ${KWECommon_INCLUDES} ${FILE})
  endif(${VARIABLE})
ENDMACRO(CHECK_INCLUDE_FILE_CONCAT)

# Need these headers for GetMacAddress in vtkKWEUUID
# Tricky as you need to figure out the proper order to test the inclusion of files
CHECK_INCLUDE_FILE_CONCAT("unistd.h"       CMAKE_HAVE_UNISTD_H)
CHECK_INCLUDE_FILE_CONCAT("stdlib.h"       CMAKE_HAVE_STDLIB_H)
if(UNIX) #Avoid polluting Win32 cmakecache
  CHECK_INCLUDE_FILE_CONCAT("sys/ioctl.h"    CMAKE_HAVE_SYS_IOCTL_H)
  CHECK_INCLUDE_FILE_CONCAT("sys/socket.h"   CMAKE_HAVE_SYS_SOCKET_H)
  CHECK_INCLUDE_FILE_CONCAT("sys/sockio.h"   CMAKE_HAVE_SYS_SOCKIO_H)
  CHECK_INCLUDE_FILE_CONCAT("net/if.h"       CMAKE_HAVE_NET_IF_H)
  CHECK_INCLUDE_FILE_CONCAT("netinet/in.h"   CMAKE_HAVE_NETINET_IN_H)
  CHECK_INCLUDE_FILE_CONCAT("net/if_dl.h"    CMAKE_HAVE_NET_IF_DL_H)
  CHECK_INCLUDE_FILE_CONCAT("net/if_arp.h"   CMAKE_HAVE_NET_IF_ARP_H)
endif(UNIX)

# Test if os defines a length for sockaddr
# only doing the test when we have sys/socket
if( ${CMAKE_HAVE_SYS_SOCKET_H} )
  if(NOT DEFINED "HAVE_SA_LEN")
    STRING(ASCII 35 POUND)
    FILE(WRITE ${PROJECT_BINARY_DIR}/CMakeTmp/kweCommonTestHAVESALEN.c
      "${POUND}include <sys/types.h>\n"
      "${POUND}include <sys/socket.h>\n"
      "int main() { struct sockaddr sa; sa.sa_len = 0; }\n")
    message(STATUS "Checking if ifreq has a sa_len")
    TRY_COMPILE(HAVE_SA_LEN
      ${PROJECT_BINARY_DIR}
      ${PROJECT_BINARY_DIR}/CMakeTmp/kweCommonTestHAVESALEN.c
      OUTPUT_VARIABLE OUTPUT)
    if(HAVE_SA_LEN)
      message(STATUS "Checking if ifreq has a sa_len -- yes")
      set(HAVE_SA_LEN 1 CACHE INTERNAL "Support if ifreq has a sa_len")
      write_file(${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
        "Determining if ifreq has a sa_len"
        "passed with the following output:\n"
        "${OUTPUT}\n" APPEND)
    else(HAVE_SA_LEN)
      message(STATUS "Checking if ifreq has a sa_len -- no")
      set(HAVE_SA_LEN 0 CACHE INTERNAL "Support if ifreq has a sa_len")
      write_file(${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
        "Determining if ifreq has a sa_len"
        "failed with the following output:\n"
        "${OUTPUT}\n" APPEND)
    endif(HAVE_SA_LEN)
  endif()
endif( ${CMAKE_HAVE_SYS_SOCKET_H} )
###############################################################################
###############################################################################

set(sources
  Model/vtkModel.cxx
  Model/vtkModelEdge.cxx
  Model/vtkModelEdgeUse.cxx
  Model/vtkModelEntity.cxx
  Model/vtkModelFace.cxx
  Model/vtkModelFaceUse.cxx
  Model/vtkModelGeometricEntity.cxx
  Model/vtkModelItem.cxx
  Model/vtkModelItemGenericIterator.cxx
  Model/vtkModelItemIterator.cxx
  Model/vtkModelItemListIterator.cxx
  Model/vtkModelLoopUse.cxx
  Model/vtkModelRegion.cxx
  Model/vtkModelShellUse.cxx
  Model/vtkModelVertex.cxx
  Model/vtkModelVertexUse.cxx
  Model/vtkUUID.cxx
  Serialize/vtkCommonSerializationHelper.cxx
  Serialize/vtkInformationKeyMap.cxx
  Serialize/vtkModelXMLParser.cxx
  Serialize/vtkRenderingSerializationHelper.cxx
  Serialize/vtkSerializableObject.cxx
  Serialize/vtkSerializationHelper.cxx
  Serialize/vtkSerializationHelperMap.cxx
  Serialize/vtkSerializer.cxx
  Serialize/vtkXMLElement.cxx)

set(classes
  DiscreteMesh
  ModelVertexClassification
  vtkDiscreteModel
  vtkDiscreteModelEdge
  vtkDiscreteModelEntity
  vtkDiscreteModelEntityGroup
  vtkDiscreteModelFace
  vtkDiscreteModelGeometricEntity
  vtkDiscreteModelRegion
  vtkDiscreteModelVertex
  vtkDiscreteModelWrapper
  vtkMergeEventData
  vtkModel3dm2DGridRepresentation
  vtkModel3dmGridRepresentation
  vtkModelBCGridRepresentation
  vtkModelGeneratedGridRepresentation
  vtkModelGridRepresentation
  vtkModelMaterial
  vtkModelUserName
  vtkSplitEventData
  vtkXMLModelReader
  vtkXMLModelWriter)

set(headers
  Model/vtkModel.h
  Model/vtkModelEdge.h
  Model/vtkModelEdgeUse.h
  Model/vtkModelEntity.h
  Model/vtkModelFace.h
  Model/vtkModelFaceUse.h
  Model/vtkModelGeometricEntity.h
  Model/vtkModelItem.h
  Model/vtkModelItemGenericIterator.h
  Model/vtkModelItemIterator.h
  Model/vtkModelItemListIterator.h
  Model/vtkModelLoopUse.h
  Model/vtkModelRegion.h
  Model/vtkModelShellUse.h
  Model/vtkModelVertex.h
  Model/vtkModelVertexUse.h
  Model/vtkUUID.h
  Serialize/vtkCommonSerializationHelper.h
  Serialize/vtkInformationKeyMap.h
  Serialize/vtkModelXMLParser.h
  Serialize/vtkRenderingSerializationHelper.h
  Serialize/vtkSerializableObject.h
  Serialize/vtkSerializationHelper.h
  Serialize/vtkSerializationHelperMap.h
  Serialize/vtkSerializer.h
  Serialize/vtkXMLElement.h

  DiscreteMeshCellIdIterator.h
  MeshClassification.h
  ModelEdgeHelper.h
  ModelParserHelper.h)

# Disable manifests for plugins on Windows so that they do not need to resolve their dependencies.
if (MSVC)
   set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")
endif(MSVC)

vtk_module_add_module(vtkSMTKDiscreteModel
  CLASSES ${classes}
  SOURCES ${sources}
  HEADERS ${headers}
  HEADERS_SUBDIR "smtk/session/discrete/kernel")
vtk_module_link(vtkSMTKDiscreteModel
  PRIVATE
    ${UUID_LIBS})
