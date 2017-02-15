//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include <pybind11/pybind11.h>
#include <utility>

namespace py = pybind11;

template <typename T, typename... Args>
using PySharedPtrClass = py::class_<T, std::shared_ptr<T>, Args...>;

#include "PybindFormat.h"
#include "PybindMeshIO.h"
#include "PybindMeshIOMoab.h"
#include "PybindMeshIOXMS.h"

#include "PybindAttributeReader.h"
#include "PybindAttributeWriter.h"
#include "PybindExportJSON.h"
#include "PybindExportMesh.h"
#include "PybindImportJSON.h"
#include "PybindImportMesh.h"
#include "PybindLogger.h"
#include "PybindModelToMesh.h"
#include "PybindOperatorLog.h"
#include "PybindReadMesh.h"
#include "PybindResourceSetReader.h"
#include "PybindResourceSetWriter.h"
#include "PybindWriteMesh.h"
#include "PybindXmlDocV1Parser.h"
#include "PybindXmlDocV2Parser.h"
#include "PybindXmlStringWriter.h"
#include "PybindXmlV2StringWriter.h"
#include "PybindXmlV3StringWriter.h"

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

PYBIND11_PLUGIN(_smtkPybindIO)
{
  py::module io("_smtkPybindIO", "<description>");
  py::module mesh = io.def_submodule("mesh", "<description>");

  // The order of these function calls is important! It was determined by
  // comparing the dependencies of each of the wrapped objects.
  PySharedPtrClass< smtk::io::mesh::Format > smtk_io_mesh_Format = pybind11_init_smtk_io_mesh_Format(mesh);
  PySharedPtrClass< smtk::io::mesh::MeshIO > smtk_io_mesh_MeshIO = pybind11_init_smtk_io_mesh_MeshIO(mesh);
  pybind11_init_smtk_io_mesh_Subset(mesh);
  PySharedPtrClass< smtk::io::mesh::MeshIOMoab > smtk_io_mesh_MeshIOMoab = pybind11_init_smtk_io_mesh_MeshIOMoab(mesh, smtk_io_mesh_MeshIO);
  PySharedPtrClass< smtk::io::mesh::MeshIOXMS > smtk_io_mesh_MeshIOXMS = pybind11_init_smtk_io_mesh_MeshIOXMS(mesh, smtk_io_mesh_MeshIO);

  PySharedPtrClass< smtk::io::AttRefInfo > smtk_io_AttRefInfo = pybind11_init_smtk_io_AttRefInfo(io);
  PySharedPtrClass< smtk::io::AttributeReader > smtk_io_AttributeReader = pybind11_init_smtk_io_AttributeReader(io);
  PySharedPtrClass< smtk::io::AttributeWriter > smtk_io_AttributeWriter = pybind11_init_smtk_io_AttributeWriter(io);
  pybind11_init_smtk_io_JSONFlags(io);
  PySharedPtrClass< smtk::io::ExportJSON > smtk_io_ExportJSON = pybind11_init_smtk_io_ExportJSON(io);
  PySharedPtrClass< smtk::io::ExportMesh > smtk_io_ExportMesh = pybind11_init_smtk_io_ExportMesh(io);
  PySharedPtrClass< smtk::io::ImportJSON > smtk_io_ImportJSON = pybind11_init_smtk_io_ImportJSON(io);
  PySharedPtrClass< smtk::io::ImportMesh > smtk_io_ImportMesh = pybind11_init_smtk_io_ImportMesh(io);
  PySharedPtrClass< smtk::io::ItemExpressionInfo > smtk_io_ItemExpressionInfo = pybind11_init_smtk_io_ItemExpressionInfo(io);
  PySharedPtrClass< smtk::io::Logger > smtk_io_Logger = pybind11_init_smtk_io_Logger(io);
  PySharedPtrClass< smtk::io::ModelToMesh > smtk_io_ModelToMesh = pybind11_init_smtk_io_ModelToMesh(io);
  PySharedPtrClass< smtk::io::OperatorLog > smtk_io_OperatorLog = pybind11_init_smtk_io_OperatorLog(io);
  PySharedPtrClass< smtk::io::ReadMesh > smtk_io_ReadMesh = pybind11_init_smtk_io_ReadMesh(io);
  PySharedPtrClass< smtk::io::ResourceSetReader > smtk_io_ResourceSetReader = pybind11_init_smtk_io_ResourceSetReader(io);
  PySharedPtrClass< smtk::io::ResourceSetWriter > smtk_io_ResourceSetWriter = pybind11_init_smtk_io_ResourceSetWriter(io);
  PySharedPtrClass< smtk::io::WriteMesh > smtk_io_WriteMesh = pybind11_init_smtk_io_WriteMesh(io);
  PySharedPtrClass< smtk::io::XmlDocV1Parser > smtk_io_XmlDocV1Parser = pybind11_init_smtk_io_XmlDocV1Parser(io);
  PySharedPtrClass< smtk::io::XmlStringWriter > smtk_io_XmlStringWriter = pybind11_init_smtk_io_XmlStringWriter(io);
  PySharedPtrClass< smtk::io::XmlV2StringWriter > smtk_io_XmlV2StringWriter = pybind11_init_smtk_io_XmlV2StringWriter(io);
  PySharedPtrClass< smtk::io::XmlV3StringWriter > smtk_io_XmlV3StringWriter = pybind11_init_smtk_io_XmlV3StringWriter(io);

  pybind11_init__ZN4smtk2io10exportMeshERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh10CollectionEEE(io);
  pybind11_init__ZN4smtk2io10exportMeshERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh10CollectionEEENSA_INS_5model7ManagerEEES9_(io);
  pybind11_init__ZN4smtk2io10importMeshERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh7ManagerEEE(io);
  pybind11_init__ZN4smtk2io10importMeshERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh7ManagerEEES9_(io);
  pybind11_init__ZN4smtk2io10importMeshERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh10CollectionEEE(io);
  pybind11_init__ZN4smtk2io10importMeshERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh10CollectionEEES9_(io);
  pybind11_init__ZN4smtk2io13readDirichletERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh7ManagerEEE(io);
  pybind11_init__ZN4smtk2io13readDirichletERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh10CollectionEEE(io);
  pybind11_init__ZN4smtk2io10readDomainERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh7ManagerEEE(io);
  pybind11_init__ZN4smtk2io10readDomainERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh10CollectionEEE(io);
  pybind11_init__ZN4smtk2io20readEntireCollectionERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh7ManagerEEE(io);
  pybind11_init__ZN4smtk2io20readEntireCollectionERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh10CollectionEEE(io);
  pybind11_init__ZN4smtk2io8readMeshERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh7ManagerEEENS0_4mesh6SubsetE(io);
  pybind11_init__ZN4smtk2io8readMeshERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh10CollectionEEENS0_4mesh6SubsetE(io);
  pybind11_init__ZN4smtk2io11readNeumannERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh7ManagerEEE(io);
  pybind11_init__ZN4smtk2io11readNeumannERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh10CollectionEEE(io);
  pybind11_init__ZN4smtk2io14writeDirichletERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh10CollectionEEE(io);
  pybind11_init__ZN4smtk2io14writeDirichletENSt3__110shared_ptrINS_4mesh10CollectionEEE(io);
  pybind11_init__ZN4smtk2io11writeDomainERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh10CollectionEEE(io);
  pybind11_init__ZN4smtk2io11writeDomainENSt3__110shared_ptrINS_4mesh10CollectionEEE(io);
  pybind11_init__ZN4smtk2io21writeEntireCollectionERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh10CollectionEEE(io);
  pybind11_init__ZN4smtk2io21writeEntireCollectionENSt3__110shared_ptrINS_4mesh10CollectionEEE(io);
  pybind11_init__ZN4smtk2io9writeMeshERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh10CollectionEEENS0_4mesh6SubsetE(io);
  pybind11_init__ZN4smtk2io9writeMeshENSt3__110shared_ptrINS_4mesh10CollectionEEENS0_4mesh6SubsetE(io);
  pybind11_init__ZN4smtk2io12writeNeumannERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh10CollectionEEE(io);
  pybind11_init__ZN4smtk2io12writeNeumannENSt3__110shared_ptrINS_4mesh10CollectionEEE(io);
  PySharedPtrClass< smtk::io::XmlDocV2Parser > smtk_io_XmlDocV2Parser = pybind11_init_smtk_io_XmlDocV2Parser(io, smtk_io_XmlDocV1Parser);

  return io.ptr();
}
