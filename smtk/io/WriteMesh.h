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

#include "smtk/io/mesh/MeshIO.h"

#include <string>
#include <vector>

/**\brief Write an entire SMTK mesh collection from a file, or just sub-sections
  *
  */

namespace smtk
{
namespace io
{

class SMTKCORE_EXPORT WriteMesh
{
public:
  WriteMesh();
  ~WriteMesh();

#ifndef SHIBOKEN_SKIP
  WriteMesh& operator=(const WriteMesh&) = delete;
  WriteMesh(const WriteMesh&) = delete;

  static std::vector<smtk::io::mesh::MeshIOPtr>& SupportedIOTypes();
#endif

  bool operator()(const std::string& filePath, smtk::mesh::CollectionPtr collection,
    mesh::Subset subset = mesh::Subset::EntireCollection) const;
  bool operator()(smtk::mesh::CollectionPtr collection,
    mesh::Subset subset = mesh::Subset::EntireCollection) const;
};

SMTKCORE_EXPORT bool writeMesh(const std::string& filePath, smtk::mesh::CollectionPtr collection,
  mesh::Subset subset = mesh::Subset::EntireCollection);
SMTKCORE_EXPORT bool writeEntireCollection(
  const std::string& filePath, smtk::mesh::CollectionPtr collection);
// Explicit functions for each subset type for Shiboken to digest
SMTKCORE_EXPORT bool writeDomain(const std::string& filePath, smtk::mesh::CollectionPtr collection);
SMTKCORE_EXPORT bool writeDirichlet(
  const std::string& filePath, smtk::mesh::CollectionPtr collection);
SMTKCORE_EXPORT bool writeNeumann(
  const std::string& filePath, smtk::mesh::CollectionPtr collection);

SMTKCORE_EXPORT bool writeMesh(
  smtk::mesh::CollectionPtr collection, mesh::Subset subset = mesh::Subset::EntireCollection);
// Explicit functions for each subset type for Shiboken to digest
SMTKCORE_EXPORT bool writeEntireCollection(smtk::mesh::CollectionPtr collection);
SMTKCORE_EXPORT bool writeDomain(smtk::mesh::CollectionPtr collection);
SMTKCORE_EXPORT bool writeDirichlet(smtk::mesh::CollectionPtr collection);
SMTKCORE_EXPORT bool writeNeumann(smtk::mesh::CollectionPtr collection);
}
}

#endif
