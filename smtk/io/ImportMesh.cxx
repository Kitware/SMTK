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
#include "smtk/mesh/moab/Readers.h"

namespace smtk {
namespace io {

smtk::mesh::CollectionPtr ImportMesh::entireFile(const std::string& filePath,
                                                 const smtk::mesh::ManagerPtr& manager)
{
  smtk::mesh::CollectionPtr collection = smtk::mesh::moab::read(filePath, manager);
  collection->readLocation(filePath);
  return collection;
}

smtk::mesh::CollectionPtr ImportMesh::onlyDomain(const std::string& filePath,
                                                 const smtk::mesh::ManagerPtr& manager)
{
 smtk::mesh::CollectionPtr collection = smtk::mesh::moab::read_domain(filePath, manager);
 collection->readLocation(filePath);
 return collection;
}


smtk::mesh::CollectionPtr ImportMesh::onlyNeumann(const std::string& filePath,
                                                  const smtk::mesh::ManagerPtr& manager)
{
  smtk::mesh::CollectionPtr collection = smtk::mesh::moab::read_neumann(filePath, manager);
  collection->readLocation(filePath);
  return collection;
}

smtk::mesh::CollectionPtr ImportMesh::onlyDirichlet(const std::string& filePath,
                                                    const smtk::mesh::ManagerPtr& manager)
{
  smtk::mesh::CollectionPtr collection = smtk::mesh::moab::read_dirichlet(filePath, manager);
  collection->readLocation(filePath);
  return collection;
}


//Merge the entire moab data file into an existing valid collection.
bool ImportMesh::entireFileToCollection(const std::string& filePath,
                                        const smtk::mesh::CollectionPtr& collection)
{
  const bool result = smtk::mesh::moab::import(filePath, collection);
  if (result && collection->readLocation().empty())
    { //if no read location is associated with this file, specify one
    collection->readLocation(filePath);
    }
  return result;
}

//Merge the domain sets from a moab data file into an existing valid collection.
bool ImportMesh::addDomainToCollection(const std::string& filePath,
                                       const smtk::mesh::CollectionPtr& collection)
{
  const bool result = smtk::mesh::moab::import_domain(filePath, collection);
  if (result && collection->readLocation().empty())
    { //if no read location is associated with this file, specify one
    collection->readLocation(filePath);
    }
  return result;
}

//Merge the neumann sets from a moab data file into an existing valid collection.
bool ImportMesh::addNeumannToCollection(const std::string& filePath,
                                        const smtk::mesh::CollectionPtr& collection)
{
  const bool result = smtk::mesh::moab::import_neumann(filePath, collection);
  if (result && collection->readLocation().empty())
    { //if no read location is associated with this file, specify one
    collection->readLocation(filePath);
    }
  return result;
}

//Merge the dirichlet sets from a moab data file into an existing valid collection.
bool ImportMesh::addDirichletToCollection(const std::string& filePath,
                                          const smtk::mesh::CollectionPtr& collection)
{
  const bool result = smtk::mesh::moab::import_dirichlet(filePath, collection);
  if (result && collection->readLocation().empty())
    { //if no read location is associated with this file, specify one
    collection->readLocation(filePath);
    }
  return result;
}

}
}
