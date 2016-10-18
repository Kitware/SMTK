//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_io_MeshIOMoab_h
#define __smtk_io_MeshIOMoab_h

#include "smtk/CoreExports.h" // For SMTKCORE_EXPORT macro.
#include "smtk/PublicPointerDefs.h"

#include "smtk/io/mesh/MeshIO.h"

#include <utility>
/**\brief Export an SMTK mesh to a file
  *
  */

namespace smtk {
  namespace io {
namespace mesh {

class SMTKCORE_EXPORT MeshIOMoab : public MeshIO
{
public:
  MeshIOMoab();

  //Load an entire moab data file as a new collection into the given manager
  //Returns an invalid collection that is NOT part of the manager if the
  //file can't be loaded. The third parameter is a label with which the domain
  //can be parsed, but it is not currently implemented for the moab interface
  smtk::mesh::CollectionPtr
    importMesh( const std::string& filePath,
                smtk::mesh::ManagerPtr& manager,
                const std::string& ) const override;

  //Merge a moab data file into an existing valid collection. The third
  //parameter is a label with which the domain can be parsed, but it is not
  //currently implemented for the moab interface
  bool importMesh( const std::string& filePath,
                   smtk::mesh::CollectionPtr collection,
                   const std::string& ) const override;

  //Exports the collection to file. Overwrites any existing content in the file
  bool exportMesh( const std::string& filePath,
                   smtk::mesh::CollectionPtr collection ) const override;

  //TODO:
  // bool exportMesh( const std::string& filePath,
  //                  smtk::mesh::CollectionPtr collection,
  //                  smtk::model::ManagerPtr manager,
  //                  const std::string& modelPropertyName ) const

  //Load an entire moab data file as a new collection into the given manager
  //Returns an invalid collection that is NOT part of the manager if the
  //file can't be loaded
  smtk::mesh::CollectionPtr
    read( const std::string& filePath,
          smtk::mesh::ManagerPtr& manager,
          Subset s) const override;

  //Merge a moab data file into an existing valid collection.
  bool read( const std::string& filePath,
             smtk::mesh::CollectionPtr collection,
             Subset s ) const override;

  //Writes the collection to file. Overwrites any existing content in the file
  bool write( const std::string& filePath,
              smtk::mesh::CollectionPtr collection,
              Subset s ) const override;

  //Writes the collection to the file specified by the collections data member
  //writeLocation(). Overwrites any existing content in the file
  bool write( smtk::mesh::CollectionPtr collection,
              Subset s ) const override;
};

}
}
}

#endif
