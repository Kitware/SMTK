//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/common/CompilerInformation.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include <pybind11/pybind11.h>
SMTK_THIRDPARTY_POST_INCLUDE

#include <utility>

namespace py = pybind11;

template <typename T, typename... Args>
using PySharedPtrClass = py::class_<T, std::shared_ptr<T>, Args...>;

#include "smtk/common/pybind11/PybindUUIDTypeCaster.h"
#include "PybindFormat.h"
#include "PybindMeshIO.h"
#include "PybindMeshIOMoab.h"
#include "PybindMeshIOXMS.h"

#include "PybindAttributeReader.h"
#include "PybindAttributeWriter.h"
#include "PybindExportMesh.h"
#include "PybindImportMesh.h"
#include "PybindLogger.h"
#include "PybindModelToMesh.h"
#include "PybindReadMesh.h"
#include "PybindWriteMesh.h"

#include "smtk/mesh/core/Resource.h"

PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>);

PYBIND11_MODULE(_smtkPybindIO, io)
{
  io.doc() = "<description>";
  py::module mesh = io.def_submodule("mesh", "<description>");

  // The order of these function calls is important! It was determined by
  // comparing the dependencies of each of the wrapped objects.
  PySharedPtrClass< smtk::io::mesh::Format > smtk_io_mesh_Format = pybind11_init_smtk_io_mesh_Format(mesh);
  PySharedPtrClass< smtk::io::mesh::MeshIO > smtk_io_mesh_MeshIO = pybind11_init_smtk_io_mesh_MeshIO(mesh);
  pybind11_init_smtk_io_mesh_Subset(mesh);
  PySharedPtrClass< smtk::io::mesh::MeshIOMoab > smtk_io_mesh_MeshIOMoab = pybind11_init_smtk_io_mesh_MeshIOMoab(mesh, smtk_io_mesh_MeshIO);
  PySharedPtrClass< smtk::io::mesh::MeshIOXMS > smtk_io_mesh_MeshIOXMS = pybind11_init_smtk_io_mesh_MeshIOXMS(mesh, smtk_io_mesh_MeshIO);

  PySharedPtrClass< smtk::io::AttributeReader > smtk_io_AttributeReader = pybind11_init_smtk_io_AttributeReader(io);
  PySharedPtrClass< smtk::io::AttributeWriter > smtk_io_AttributeWriter = pybind11_init_smtk_io_AttributeWriter(io);
  PySharedPtrClass< smtk::io::ExportMesh > smtk_io_ExportMesh = pybind11_init_smtk_io_ExportMesh(io);
  PySharedPtrClass< smtk::io::ImportMesh > smtk_io_ImportMesh = pybind11_init_smtk_io_ImportMesh(io);
  PySharedPtrClass< smtk::io::Logger > smtk_io_Logger = pybind11_init_smtk_io_Logger(io);
  PySharedPtrClass< smtk::io::ModelToMesh > smtk_io_ModelToMesh = pybind11_init_smtk_io_ModelToMesh(io);
  PySharedPtrClass< smtk::io::ReadMesh > smtk_io_ReadMesh = pybind11_init_smtk_io_ReadMesh(io);
  PySharedPtrClass< smtk::io::WriteMesh > smtk_io_WriteMesh = pybind11_init_smtk_io_WriteMesh(io);

  pybind11_init__ZN4smtk2io10exportMeshERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh10ResourceEEE(io);
  pybind11_init__ZN4smtk2io10exportMeshERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh10ResourceEEENSA_INS_5model7ResourceEEES9_(io);
  pybind11_init__ZN4smtk2io10importMeshERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh7InterfaceEEE(io);
  pybind11_init__ZN4smtk2io10importMeshERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh7InterfaceEEES9_(io);
  pybind11_init__ZN4smtk2io10importMeshERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh10ResourceEEE(io);
  pybind11_init__ZN4smtk2io10importMeshERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh10ResourceEEES9_(io);
  pybind11_init__ZN4smtk2io13readDirichletERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh7InterfaceEEE(io);
  pybind11_init__ZN4smtk2io13readDirichletERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh10ResourceEEE(io);
  pybind11_init__ZN4smtk2io10readDomainERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh7InterfaceEEE(io);
  pybind11_init__ZN4smtk2io10readDomainERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh10ResourceEEE(io);
  pybind11_init__ZN4smtk2io20readEntireResourceERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh7InterfaceEEE(io);
  pybind11_init__ZN4smtk2io20readEntireResourceERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh10ResourceEEE(io);
  pybind11_init__ZN4smtk2io8readMeshERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh7InterfaceEEENS0_4mesh6SubsetE(io);
  pybind11_init__ZN4smtk2io8readMeshERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh10ResourceEEENS0_4mesh6SubsetE(io);
  pybind11_init__ZN4smtk2io11readNeumannERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh7InterfaceEEE(io);
  pybind11_init__ZN4smtk2io11readNeumannERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh10ResourceEEE(io);
  pybind11_init__ZN4smtk2io14writeDirichletERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh10ResourceEEE(io);
  pybind11_init__ZN4smtk2io14writeDirichletENSt3__110shared_ptrINS_4mesh10ResourceEEE(io);
  pybind11_init__ZN4smtk2io11writeDomainERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh10ResourceEEE(io);
  pybind11_init__ZN4smtk2io11writeDomainENSt3__110shared_ptrINS_4mesh10ResourceEEE(io);
  pybind11_init__ZN4smtk2io21writeEntireResourceERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh10ResourceEEE(io);
  pybind11_init__ZN4smtk2io21writeEntireResourceENSt3__110shared_ptrINS_4mesh10ResourceEEE(io);
  pybind11_init__ZN4smtk2io9writeMeshERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh10ResourceEEENS0_4mesh6SubsetE(io);
  pybind11_init__ZN4smtk2io9writeMeshENSt3__110shared_ptrINS_4mesh10ResourceEEENS0_4mesh6SubsetE(io);
  pybind11_init__ZN4smtk2io12writeNeumannERKNSt3__112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEENS1_10shared_ptrINS_4mesh10ResourceEEE(io);
  pybind11_init__ZN4smtk2io12writeNeumannENSt3__110shared_ptrINS_4mesh10ResourceEEE(io);
}
