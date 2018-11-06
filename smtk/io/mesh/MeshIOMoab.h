//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_io_MeshIOMoab_h
#define __smtk_io_MeshIOMoab_h

#include "smtk/CoreExports.h" // For SMTKCORE_EXPORT macro.
#include "smtk/PublicPointerDefs.h"

#include "smtk/io/mesh/MeshIO.h"

#include <utility>
/**\brief Export an SMTK mesh to a file
  *
  */

namespace smtk
{
namespace io
{
namespace mesh
{

class SMTKCORE_EXPORT MeshIOMoab : public MeshIO
{
public:
  MeshIOMoab();

  //Load an entire moab data file as a new resource with the given interface.
  //Returns an invalid resource if the file can't be loaded. The third
  //parameter is a label with which the domain can be parsed, but it is not
  //currently implemented for the moab interface
  smtk::mesh::ResourcePtr importMesh(const std::string& filePath,
    const smtk::mesh::InterfacePtr& interface, const std::string&) const override;

  //Merge a moab data file into an existing valid resource. The third
  //parameter is a label with which the domain can be parsed, but it is not
  //currently implemented for the moab interface
  bool importMesh(const std::string& filePath, smtk::mesh::ResourcePtr resource,
    const std::string&) const override;

  //Exports the resource to file. Overwrites any existing content in the file
  bool exportMesh(const std::string& filePath, smtk::mesh::ResourcePtr resource) const override;

  //TODO:
  // bool exportMesh( const std::string& filePath,
  //                  smtk::mesh::ResourcePtr meshResource,
  //                  smtk::model::ResourcePtr modelResource,
  //                  const std::string& modelPropertyName ) const

  //Load an entire moab data file as a new resource with the given interface.
  //Returns an invalid resource if the file can't be loaded
  smtk::mesh::ResourcePtr read(const std::string& filePath,
    const smtk::mesh::InterfacePtr& interface, Subset s) const override;

  //Merge a moab data file into an existing valid resource.
  bool read(const std::string& filePath, smtk::mesh::ResourcePtr resource, Subset s) const override;

  //Writes the resource to file. Overwrites any existing content in the file
  bool write(
    const std::string& filePath, smtk::mesh::ResourcePtr resource, Subset s) const override;

  //Writes the resource to the file specified by the resources data member
  //writeLocation(). Overwrites any existing content in the file
  bool write(smtk::mesh::ResourcePtr resource, Subset s) const override;
};
}
}
}

#endif
