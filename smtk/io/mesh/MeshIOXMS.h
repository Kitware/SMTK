//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_io_MeshIOXMS_h
#define __smtk_io_MeshIOXMS_h

#include "smtk/CoreExports.h" // For SMTKCORE_EXPORT macro.
#include "smtk/PublicPointerDefs.h"

#include "smtk/io/mesh/MeshIO.h"

#include "smtk/mesh/DimensionTypes.h"

#include <ostream>
#include <string>
/**\brief Export a SMTK mesh to a file in the XMS format.
  *
  *
  */

namespace smtk
{
namespace io
{
namespace mesh
{

class SMTKCORE_EXPORT MeshIOXMS : public MeshIO
{
public:
  MeshIOXMS();

  //Returns True if and only if a file has been written to disk.
  //Occurrences that can cause write to fail:
  // Collection is empty
  // Collection has no Triangles or Quads
  bool exportMesh(std::ostream& stream, smtk::mesh::CollectionPtr collection,
    smtk::mesh::DimensionType dim) const;

  bool exportMesh(const std::string& filePath, smtk::mesh::CollectionPtr collection,
    smtk::mesh::DimensionType dim) const;

  bool exportMesh(const std::string& filePath, smtk::mesh::CollectionPtr collection) const override;

  bool exportMesh(std::ostream& stream, smtk::mesh::CollectionPtr collection,
    smtk::model::ManagerPtr manager, const std::string& modelPropertyName,
    smtk::mesh::DimensionType dim) const;

  bool exportMesh(const std::string& filePath, smtk::mesh::CollectionPtr collection,
    smtk::model::ManagerPtr manager, const std::string& modelPropertyName,
    smtk::mesh::DimensionType dim) const;

  bool exportMesh(const std::string& filePath, smtk::mesh::CollectionPtr collection,
    smtk::model::ManagerPtr manager, const std::string& modelPropertyName) const override;
};
}
}
}

#endif
