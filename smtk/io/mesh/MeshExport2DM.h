//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_io_MeshExport2DM_h
#define __smtk_io_MeshExport2DM_h

#include "smtk/CoreExports.h" // For SMTKCORE_EXPORT macro.
#include "smtk/PublicPointerDefs.h"


#include <string>
#include <ostream>
/**\brief Export the 2D elements of a SMTK mesh to a file in the 2DM format.
  *
  *
  */

namespace smtk {
  namespace io {

class SMTKCORE_EXPORT MeshExport2DM
{
public:
  MeshExport2DM();

  //Returns True if and only if a file has been written to disk.
  //Occurrences that can cause write to fail:
  // Collection is empty
  // Collection has no Triangles or Quads
  bool write(smtk::mesh::CollectionPtr collection,
             const std::string& filePath) const;

  bool write(smtk::mesh::CollectionPtr collection,
             std::ostream& stream) const;

  bool write(smtk::mesh::CollectionPtr collection,
             smtk::model::ManagerPtr manager,
             const std::string& modelPropertyName,
             const std::string& filePath) const;

  bool write(smtk::mesh::CollectionPtr collection,
             smtk::model::ManagerPtr manager,
             const std::string& modelPropertyName,
             std::ostream& stream) const;

};


  }
}

#endif
