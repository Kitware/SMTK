//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/vtk/io/mesh/MeshIOVTK.h"

#include "smtk/io/ExportMesh.h"
#include "smtk/io/ImportMesh.h"
#include "smtk/io/mesh/Format.h"
#include "smtk/io/mesh/MeshIO.h"

#include "smtk/mesh/core/Collection.h"
#include "smtk/mesh/core/Manager.h"

#include "smtk/extension/vtk/io/ImportAsVTKData.h"
#include "smtk/extension/vtk/io/mesh/ExportVTKData.h"
#include "smtk/extension/vtk/io/mesh/ImportVTKData.h"

#include <string>

namespace smtk
{
namespace extension
{
namespace vtk
{
namespace io
{
namespace mesh
{

MeshIOVTK::MeshIOVTK()
  : MeshIO()
{
  this->Formats.push_back(
    smtk::io::mesh::Format("vtk unstructured grid", std::vector<std::string>({ ".vtu" }),
      smtk::io::mesh::Format::Import | smtk::io::mesh::Format::Export));
  this->Formats.push_back(
    smtk::io::mesh::Format("vtk polydata", std::vector<std::string>({ ".vtp" }),
      smtk::io::mesh::Format::Import | smtk::io::mesh::Format::Export));
  this->Formats.push_back(smtk::io::mesh::Format("vtk legacy", std::vector<std::string>({ ".vtk" }),
    smtk::io::mesh::Format::Import | smtk::io::mesh::Format::Export));
  smtk::extension::vtk::io::ImportAsVTKData import;
  auto formats = import.fileFormats();
  for (auto& format : formats)
  {
    std::vector<std::string> extensionVector;
    for (auto ext : format.Extensions)
    {
      extensionVector.push_back("." + ext);
    }
    this->Formats.push_back(
      smtk::io::mesh::Format(format.Name, extensionVector, smtk::io::mesh::Format::Import));
  }
}

smtk::mesh::CollectionPtr MeshIOVTK::importMesh(const std::string& filePath,
  smtk::mesh::ManagerPtr& manager, const std::string& domainPropertyName) const
{
  smtk::extension::vtk::io::mesh::ImportVTKData import;
  return import(filePath, manager, domainPropertyName);
}

bool MeshIOVTK::importMesh(const std::string& filePath, smtk::mesh::CollectionPtr collection,
  const std::string& domainPropertyName) const
{
  smtk::extension::vtk::io::mesh::ImportVTKData import;
  return import(filePath, collection, domainPropertyName);
}

bool MeshIOVTK::exportMesh(const std::string& filePath, smtk::mesh::CollectionPtr collection) const
{
  smtk::extension::vtk::io::mesh::ExportVTKData export_;
  return export_(filePath, collection, "");
}
}
}
}
}
}

void smtk_extension_vtk_io_mesh_MeshIOVTK_AutoInit_Construct()
{
  smtk::io::ImportMesh::SupportedIOTypes().push_back(
    smtk::io::mesh::MeshIOPtr(new smtk::extension::vtk::io::mesh::MeshIOVTK()));
  smtk::io::ExportMesh::SupportedIOTypes().push_back(
    smtk::io::mesh::MeshIOPtr(new smtk::extension::vtk::io::mesh::MeshIOVTK()));
}

void smtk_extension_vtk_io_mesh_MeshIOVTK_AutoInit_Destruct()
{
  auto is_MeshIOVTK = [](const smtk::io::mesh::MeshIOPtr& meshIOPtr) {
    return dynamic_cast<smtk::extension::vtk::io::mesh::MeshIOVTK*>(meshIOPtr.get()) != nullptr;
  };

  smtk::io::ImportMesh::SupportedIOTypes().erase(
    std::remove_if(smtk::io::ImportMesh::SupportedIOTypes().begin(),
      smtk::io::ImportMesh::SupportedIOTypes().end(), is_MeshIOVTK));
  smtk::io::ExportMesh::SupportedIOTypes().erase(
    std::remove_if(smtk::io::ExportMesh::SupportedIOTypes().begin(),
      smtk::io::ExportMesh::SupportedIOTypes().end(), is_MeshIOVTK));
}

smtkComponentInitMacro(smtk_extension_vtk_io_mesh_MeshIOVTK);
