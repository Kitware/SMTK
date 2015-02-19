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

#include "smtk/SMTKCoreExports.h" // For SMTKCORE_EXPORT macro.
#include "smtk/PublicPointerDefs.h"

#include "smtk/common/UUID.h"

#include <utility>
/**\brief Import an entire SMTK mesh collection from a file, or just sub-sections
  *
  */

namespace smtk {
  namespace io {

class SMTKCORE_EXPORT ImportMesh
{
public:
  //Load an entire moab data file as a new collection into the given manager
  //Returns an invalid collection that is NOT part of the manager if the
  //file can't be loaded
  static smtk::mesh::CollectionPtr entireFile(const std::string& filePath,
                                              const smtk::mesh::ManagerPtr& manager);

  //Load the material sets from a moab data file as a new collection into the
  //given manager.
  //Returns an invalid collection that is NOT part of the manager if the
  //file can't be loaded, or no material meshes exist in the file
  static smtk::mesh::CollectionPtr onlyMaterial(const std::string& filePath,
                                                const smtk::mesh::ManagerPtr& manager);

  //Load the neumann sets from a moab data file as a new collection into the
  //given manager.
  //Returns an invalid collection that is NOT part of the manager if the
  //file can't be loaded, or no neumann meshes exist in the file
  static smtk::mesh::CollectionPtr onlyNeumann(const std::string& filePath,
                                               const smtk::mesh::ManagerPtr& manager);

  //Load the dirichlet sets from a moab data file as a new collection into the
  //given manager.
  //Returns an invalid collection that is NOT part of the manager if the
  //file can't be loaded, or no dirichlet meshes exist in the file
  static smtk::mesh::CollectionPtr onlyDirichlet(const std::string& filePath,
                                                 const smtk::mesh::ManagerPtr& manager);

  //Merge the entire moab data file into an existing valid collection.
  static bool entireFileToCollection(const std::string& filePath,
                                     const smtk::mesh::CollectionPtr& collection);

  //Merge the material sets from a moab data file into an existing valid collection.
  static bool addMaterialToCollection(const std::string& filePath,
                                       const smtk::mesh::CollectionPtr& collection);

  //Merge the neumann sets from a moab data file into an existing valid collection.
  static bool addNeumannToCollection(const std::string& filePath,
                                     const smtk::mesh::CollectionPtr& collection);

  //Merge the dirichlet sets from a moab data file into an existing valid collection.
  static bool addDirichletToCollection(const std::string& filePath,
                                       const smtk::mesh::CollectionPtr& collection);


};

  }
}

#endif
