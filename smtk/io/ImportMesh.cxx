//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/io/ImportMesh.h"

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"
#include "smtk/mesh/moab/Interface.h"

namespace smtk {
  namespace io {

/**\brief Create a collection of meshes in the \a manager given a moab file.
  *
  */
smtk::common::UUID ImportMesh::intoManager(const std::string& filePath,
                                           smtk::mesh::ManagerPtr manager)
{
  smtk::mesh::moab::InterfacePtr moab = smtk::mesh::moab::make_interface(filePath);
  if(!moab)
    {
    return smtk::common::UUID();
    }
  //make a moab specific mesh collection
  return manager->makeCollection(moab)->entity();
}

}
}
