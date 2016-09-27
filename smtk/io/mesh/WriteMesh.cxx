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

bool WriteMesh::entireCollection(smtk::mesh::CollectionPtr collection)
{
  if(collection->writeLocation().empty())
  { //require a file location to write too
    return false;
  }
  return smtk::mesh::moab::write(collection->writeLocation().absolutePath(), collection);
}

bool WriteMesh::onlyDomain(smtk::mesh::CollectionPtr collection)
{
  if(collection->writeLocation().empty())
  { //require a file location to write too
    return false;
  }
  return smtk::mesh::moab::write_domain(collection->writeLocation().absolutePath(), collection);
}

bool WriteMesh::onlyNeumann(smtk::mesh::CollectionPtr collection)
{
  if(collection->writeLocation().empty())
  { //require a file location to write too
    return false;
  }
  return smtk::mesh::moab::write_neumann(collection->writeLocation().absolutePath(), collection);
}

bool WriteMesh::onlyDirichlet(smtk::mesh::CollectionPtr collection)
{
  if(collection->writeLocation().empty())
  { //require a file location to write too
    return false;
  }
  return smtk::mesh::moab::write_dirichlet(collection->writeLocation().absolutePath(), collection);
}

bool WriteMesh::entireCollection(const std::string& filePath,
                             smtk::mesh::CollectionPtr collection)
{
  return smtk::mesh::moab::write(filePath, collection);
}

bool WriteMesh::onlyDomain(const std::string& filePath,
                         smtk::mesh::CollectionPtr collection)
{
  return smtk::mesh::moab::write_domain(filePath, collection);
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
