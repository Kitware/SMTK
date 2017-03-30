//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_extensions_vtk_io_MeshIOVTK_h
#define __smtk_extensions_vtk_io_MeshIOVTK_h

#include "smtk/AutoInit.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/extension/vtk/io/IOVTKExports.h"

#include "smtk/io/mesh/MeshIO.h"

#include <utility>
/**\brief Export an SMTK mesh to a file
  *
  */

namespace smtk
{
namespace extension
{
namespace vtk
{
namespace io
{

class SMTKIOVTK_EXPORT MeshIOVTK : public smtk::io::mesh::MeshIO
{
public:
  // smtkAutoInitComponentMacro(smtk_extension_vtk_io_MeshIOVTK)
  MeshIOVTK();

  //Load a vtk XML data file as a new collection into the given manager
  //Returns an invalid collection that is NOT part of the manager if the
  //file can't be loaded
  smtk::mesh::CollectionPtr importMesh(const std::string& filePath, smtk::mesh::ManagerPtr& manager,
    const std::string& domainPropertyName) const override;

  //Merge a vtk data file into an existing valid collection.
  bool importMesh(const std::string& filePath, smtk::mesh::CollectionPtr collection,
    const std::string& domainPropertyName) const override;

  //Epxort a collection to a VTK XML unstructured grid or polydata.
  bool exportMesh(const std::string& filePath, smtk::mesh::CollectionPtr collection) const override;
};
}
}
}
}

void SMTKIOVTK_EXPORT smtk_extension_vtk_io_MeshIOVTK_AutoInit_Construct();
void SMTKIOVTK_EXPORT smtk_extension_vtk_io_MeshIOVTK_AutoInit_Destruct();

#endif
