//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/io/WriteMesh.h"

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"
#include "smtk/mesh/moab/Writers.h"

namespace smtk {
  namespace io {

bool WriteMesh::entireCollection(const std::string& filePath,
                             smtk::mesh::CollectionPtr collection)
{
  return smtk::mesh::moab::write(filePath, collection);
}

bool WriteMesh::onlyBoundary(const std::string& filePath,
                         smtk::mesh::CollectionPtr collection)
{
  return smtk::mesh::moab::write_boundary(filePath, collection);
}

bool WriteMesh::onlyNeumann(const std::string& filePath,
                        smtk::mesh::CollectionPtr collection)
{
  return smtk::mesh::moab::write_neumann(filePath, collection);
}

bool WriteMesh::onlyDirichlet(const std::string& filePath,
                          smtk::mesh::CollectionPtr collection)
{
  return smtk::mesh::moab::write_dirichlet(filePath, collection);
}


}
}
