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

namespace
{

smtk::mesh::CollectionPtr verifyAndMake(const smtk::mesh::moab::InterfacePtr interface,
                                        const smtk::mesh::ManagerPtr manager)
{
  if(!interface)
    {
    //create an invalid collection which isn't part of a manager
    return smtk::mesh::Collection::create();
    }
  //make a moab specific mesh collection
  return manager->makeCollection( interface );
}

}

smtk::mesh::CollectionPtr ImportMesh::entireFile(const std::string& filePath,
                                                 const smtk::mesh::ManagerPtr& manager)
{
  return verifyAndMake( smtk::mesh::moab::make_interface(filePath),
                        manager);

}

smtk::mesh::CollectionPtr ImportMesh::onlyBoundary(const std::string& filePath,
                                                   const smtk::mesh::ManagerPtr& manager)
{
  return verifyAndMake( smtk::mesh::moab::make_boundary_interface(filePath),
                        manager);
}


smtk::mesh::CollectionPtr ImportMesh::onlyNeumann(const std::string& filePath,
                                                  const smtk::mesh::ManagerPtr& manager)
{
  return verifyAndMake( smtk::mesh::moab::make_neumann_interface(filePath),
                        manager);
}

smtk::mesh::CollectionPtr ImportMesh::onlyDirichlet(const std::string& filePath,
                                                    const smtk::mesh::ManagerPtr& manager)
{
  return verifyAndMake( smtk::mesh::moab::make_dirichlet_interface(filePath),
                        manager);
}


//Merge the entire moab data file into an existing valid collection.
bool ImportMesh::entireFileToCollection(const std::string& filePath,
                                        const smtk::mesh::CollectionPtr& collection)
{
  return smtk::mesh::moab::import(filePath, collection);
}

//Merge the boundary sets from a moab data file into an existing valid collection.
bool ImportMesh::addBoundaryToCollection(const std::string& filePath,
                                         const smtk::mesh::CollectionPtr& collection)
{
  return smtk::mesh::moab::import_boundary(filePath, collection);
}

//Merge the neumann sets from a moab data file into an existing valid collection.
bool ImportMesh::addNeumannToCollection(const std::string& filePath,
                                        const smtk::mesh::CollectionPtr& collection)
{
  return smtk::mesh::moab::import_neumann(filePath, collection);
}

//Merge the dirichlet sets from a moab data file into an existing valid collection.
bool ImportMesh::addDirichletToCollection(const std::string& filePath,
                                          const smtk::mesh::CollectionPtr& collection)
{
  return smtk::mesh::moab::import_dirichlet(filePath, collection);
}

}
}
