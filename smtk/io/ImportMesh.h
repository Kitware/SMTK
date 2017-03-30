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

#include "smtk/CoreExports.h" // For SMTKCORE_EXPORT macro.
#include "smtk/PublicPointerDefs.h"

#include "smtk/io/mesh/MeshIO.h"

#include <string>
#include <vector>

/**\brief Import an entire SMTK mesh collection from a file, or just sub-sections
  *
  */

namespace smtk
{
namespace io
{

class SMTKCORE_EXPORT ImportMesh
{
public:
  ImportMesh();
  ~ImportMesh();

#ifndef SHIBOKEN_SKIP
  ImportMesh& operator=(const ImportMesh&) = delete;
  ImportMesh(const ImportMesh&) = delete;

  static std::vector<smtk::io::mesh::MeshIOPtr>& SupportedIOTypes();

  static bool ExtensionIsSupported(const std::string& ext);

  //Load the domain sets from a moab data file as a new collection into the
  //given manager.
  smtk::mesh::CollectionPtr operator()(const std::string& filePath, smtk::mesh::ManagerPtr manager,
    std::string domainPropertyName = std::string()) const;
  bool operator()(const std::string& filePath, smtk::mesh::CollectionPtr collection,
    std::string domainPropertyName = std::string()) const;
#endif
};

SMTKCORE_EXPORT smtk::mesh::CollectionPtr importMesh(
  const std::string& filePath, smtk::mesh::ManagerPtr manager);
SMTKCORE_EXPORT smtk::mesh::CollectionPtr importMesh(const std::string& filePath,
  smtk::mesh::ManagerPtr manager, const std::string& domainPropertyName);
SMTKCORE_EXPORT bool importMesh(const std::string& filePath, smtk::mesh::CollectionPtr collection);
SMTKCORE_EXPORT bool importMesh(const std::string& filePath, smtk::mesh::CollectionPtr collection,
  const std::string& domainPropertyName);
}
}

#endif
