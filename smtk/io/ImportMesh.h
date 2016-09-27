//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_io_ImportMesh_h
#define __smtk_io_ImportMesh_h

#include "smtk/CoreExports.h" // For SMTKCORE_EXPORT macro.
#include "smtk/PublicPointerDefs.h"

#include <string>
#include <vector>

/**\brief Import an entire SMTK mesh collection from a file, or just sub-sections
  *
  */

namespace smtk {
  namespace io {

namespace mesh {
class MeshIO;
}

class SMTKCORE_EXPORT ImportMesh
{
public:
  ImportMesh();
  ~ImportMesh();
  //Load the domain sets from a moab data file as a new collection into the
  //given manager.
  smtk::mesh::CollectionPtr operator() (const std::string& filePath,
                                        smtk::mesh::ManagerPtr manager) const;
  bool operator() (const std::string& filePath,
                   smtk::mesh::CollectionPtr collection) const;

  protected:
  std::vector<smtk::io::mesh::MeshIO*> IO;
};

}
}

#endif
