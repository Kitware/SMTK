//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_io_WriteMesh_h
#define __smtk_io_WriteMesh_h

#include "smtk/CoreExports.h" // For SMTKCORE_EXPORT macro.
#include "smtk/PublicPointerDefs.h"

#include "smtk/common/UUID.h"

#include <utility>
/**\brief Export an SMTK mesh from a file
  *
  */

namespace smtk {
  namespace io {

class SMTKCORE_EXPORT WriteMesh
{
public:

  //Saves the entire collection to the file specified by the collections
  //data member writeLocation(). Overwrites any existing content
  //in the file
  static bool entireCollection(smtk::mesh::CollectionPtr collection);

  //Saves the only the domain elements of a collection to the file specified by
  //the collections data member writeLocation(). Overwrites any existing content
  //in the file
  static bool onlyDomain(smtk::mesh::CollectionPtr collection);

  //Saves the only the neumann elements of a collection to the file specified by
  //the collections data member writeLocation(). Overwrites any existing content
  //in the file
  static bool onlyNeumann(smtk::mesh::CollectionPtr collection);

  //Saves the only the dirichlet elements of a collection to the file specified by
  //the collections data member writeLocation(). Overwrites any existing content
  //in the file
  static bool onlyDirichlet(smtk::mesh::CollectionPtr collection);

  //Saves the entire collection to File. Overwrites any existing content
  //in the file
  static bool entireCollection(const std::string& filePath,
                               smtk::mesh::CollectionPtr collection);

  //Saves the only the domain elements of a collection to File.
  //Overwrites any existing content in the file
  static bool onlyDomain(const std::string& filePath,
                         smtk::mesh::CollectionPtr collection);

  //Saves the only the neumann elements of a collection to File.
  //Overwrites any existing content in the file
  static bool onlyNeumann(const std::string& filePath,
                          smtk::mesh::CollectionPtr collection);

  //Saves the only the dirichlet elements of a collection to File.
  //Overwrites any existing content in the file
  static bool onlyDirichlet(const std::string& filePath,
                            smtk::mesh::CollectionPtr collection);
};

  }
}

#endif
