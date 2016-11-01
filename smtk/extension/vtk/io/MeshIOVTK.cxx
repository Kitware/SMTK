//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/vtk/io/MeshIOVTK.h"

#include "smtk/io/ImportMesh.h"
#include "smtk/io/mesh/Format.h"
#include "smtk/io/mesh/MeshIO.h"

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"

#include "smtk/extension/vtk/io/ExportVTKData.h"
#include "smtk/extension/vtk/io/ImportVTKData.h"

#include <string>

namespace smtk {
namespace extension {
namespace vtk {
namespace io {

MeshIOVTK::MeshIOVTK() : MeshIO()
{
  this->Formats.push_back(
    smtk::io::mesh::Format("vtk unstructured grid",
                           std::vector<std::string>({ ".vtu" }),
                           smtk::io::mesh::Format::Import |
                           smtk::io::mesh::Format::Export));
  this->Formats.push_back(
    smtk::io::mesh::Format("vtk polydata",
                           std::vector<std::string>({ ".vtp" }),
                           smtk::io::mesh::Format::Import |
                           smtk::io::mesh::Format::Export));
}

smtk::mesh::CollectionPtr
MeshIOVTK::importMesh( const std::string& filePath,
                        smtk::mesh::ManagerPtr& manager,
                       const std::string& domainPropertyName ) const
{
  smtk::extension::vtk::io::ImportVTKData import;
  return import( filePath, manager, domainPropertyName );
}

bool MeshIOVTK::importMesh( const std::string& filePath,
                            smtk::mesh::CollectionPtr collection,
                            const std::string& domainPropertyName ) const
{
  smtk::extension::vtk::io::ImportVTKData import;
  return import( filePath, collection, domainPropertyName );
}

bool MeshIOVTK::exportMesh( const std::string& filePath,
                            smtk::mesh::CollectionPtr collection) const
{
  smtk::extension::vtk::io::ExportVTKData export_;
  return export_( filePath, collection, "" );
}


}
}
}
}

void smtk_extension_vtk_io_MeshIOVTK_AutoInit_Construct()
{
  smtk::io::ImportMesh::SupportedIOTypes().push_back(
  smtk::io::mesh::MeshIOPtr( new smtk::extension::vtk::io::MeshIOVTK() ) );
}

void smtk_extension_vtk_io_MeshIOVTK_AutoInit_Destruct()
{
}

smtkComponentInitMacro(smtk_extension_vtk_io_MeshIOVTK);
