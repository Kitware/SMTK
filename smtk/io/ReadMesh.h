//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_io_ReadMesh_h
#define __smtk_io_ReadMesh_h

#include "smtk/CoreExports.h" // For SMTKCORE_EXPORT macro.
#include "smtk/PublicPointerDefs.h"

#include "smtk/io/mesh/MeshIO.h"

#include <string>
#include <vector>

/**\brief Read an entire SMTK mesh collection from a file, or just sub-sections
  *
  */

namespace smtk {
  namespace io {

class SMTKCORE_EXPORT ReadMesh
{
public:
  ReadMesh();
  ~ReadMesh();
  //Load the domain sets from a moab data file as a new collection into the
  //given manager.
  smtk::mesh::CollectionPtr
    operator() (const std::string& filePath,
                smtk::mesh::ManagerPtr manager,
                mesh::Subset subset = mesh::Subset::EntireCollection) const;
  bool operator() (const std::string& filePath,
                   smtk::mesh::CollectionPtr collection,
                   mesh::Subset subset = mesh::Subset::EntireCollection) const;

  protected:
  std::vector<smtk::io::mesh::MeshIO*> IO;
};

}
}

#endif
