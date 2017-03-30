//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_io_ReadMesh_h
#define __smtk_io_ReadMesh_h

#include "smtk/CoreExports.h" // For SMTKCORE_EXPORT macro.
#include "smtk/PublicPointerDefs.h"

#include "smtk/io/mesh/MeshIO.h"

#include <string>
#include <vector>

/**\brief Read an entire SMTK mesh collection from a file, or just sub-sections
  *
  */

namespace smtk {
  namespace io {

class SMTKCORE_EXPORT ReadMesh
{
public:
  ReadMesh();
  ~ReadMesh();

#ifndef SHIBOKEN_SKIP
  ReadMesh& operator=(const ReadMesh&) = delete;
  ReadMesh(const ReadMesh&) = delete;

  static std::vector<smtk::io::mesh::MeshIOPtr>& SupportedIOTypes();

  static bool ExtensionIsSupported(const std::string& ext);
#endif

  //Load the domain sets from a moab data file as a new collection into the
  //given manager.
  smtk::mesh::CollectionPtr
    operator() (const std::string& filePath,
                smtk::mesh::ManagerPtr manager,
                mesh::Subset subset = mesh::Subset::EntireCollection) const;
  bool operator() (const std::string& filePath,
                   smtk::mesh::CollectionPtr collection,
                   mesh::Subset subset = mesh::Subset::EntireCollection) const;
};

SMTKCORE_EXPORT smtk::mesh::CollectionPtr
readMesh( const std::string& filePath,
          smtk::mesh::ManagerPtr manager,
          mesh::Subset subset = mesh::Subset::EntireCollection );
// Explicit functions for each subset type for Shiboken to digest
SMTKCORE_EXPORT smtk::mesh::CollectionPtr
readEntireCollection( const std::string& filePath,
                      smtk::mesh::ManagerPtr manager );
SMTKCORE_EXPORT smtk::mesh::CollectionPtr
readDomain( const std::string& filePath,
            smtk::mesh::ManagerPtr manager );
SMTKCORE_EXPORT smtk::mesh::CollectionPtr
readDirichlet( const std::string& filePath,
               smtk::mesh::ManagerPtr manager );
SMTKCORE_EXPORT smtk::mesh::CollectionPtr
readNeumann( const std::string& filePath,
               smtk::mesh::ManagerPtr manager );

SMTKCORE_EXPORT bool
readMesh( const std::string& filePath,
          smtk::mesh::CollectionPtr collection,
          mesh::Subset subset = mesh::Subset::EntireCollection );
// Explicit functions for each subset type for Shiboken to digest
SMTKCORE_EXPORT bool
readEntireCollection( const std::string& filePath,
                      smtk::mesh::CollectionPtr collection );
SMTKCORE_EXPORT bool
readDomain( const std::string& filePath,
            smtk::mesh::CollectionPtr collection );
SMTKCORE_EXPORT bool
readDirichlet( const std::string& filePath,
               smtk::mesh::CollectionPtr collection );
SMTKCORE_EXPORT bool
readNeumann( const std::string& filePath,
             smtk::mesh::CollectionPtr collection );

}
}

#endif
