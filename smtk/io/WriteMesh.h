//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_io_WriteMesh_h
#define smtk_io_WriteMesh_h

#include "smtk/CoreExports.h" // For SMTKCORE_EXPORT macro.
#include "smtk/PublicPointerDefs.h"

#include "smtk/io/mesh/MeshIO.h"

#include <string>
#include <vector>

namespace smtk
{
namespace io
{

/**\brief Write an entire SMTK mesh resource from a file, or just sub-sections
  *
  */
class SMTKCORE_EXPORT WriteMesh
{
public:
  WriteMesh();
  ~WriteMesh();

  WriteMesh& operator=(const WriteMesh&) = delete;
  WriteMesh(const WriteMesh&) = delete;

  static std::vector<smtk::io::mesh::MeshIOPtr>& SupportedIOTypes();

  bool operator()(
    const std::string& filePath,
    smtk::mesh::ResourcePtr resource,
    mesh::Subset subset = mesh::Subset::EntireResource) const;
  bool operator()(
    smtk::mesh::ResourcePtr resource,
    mesh::Subset subset = mesh::Subset::EntireResource) const;
};

SMTKCORE_EXPORT bool writeMesh(
  const std::string& filePath,
  smtk::mesh::ResourcePtr resource,
  mesh::Subset subset = mesh::Subset::EntireResource);
SMTKCORE_EXPORT bool writeEntireResource(
  const std::string& filePath,
  smtk::mesh::ResourcePtr resource);
// Explicit functions for each subset type
SMTKCORE_EXPORT bool writeDomain(const std::string& filePath, smtk::mesh::ResourcePtr resource);
SMTKCORE_EXPORT bool writeDirichlet(const std::string& filePath, smtk::mesh::ResourcePtr resource);
SMTKCORE_EXPORT bool writeNeumann(const std::string& filePath, smtk::mesh::ResourcePtr resource);

SMTKCORE_EXPORT bool writeMesh(
  smtk::mesh::ResourcePtr resource,
  mesh::Subset subset = mesh::Subset::EntireResource);
// Explicit functions for each subset type
SMTKCORE_EXPORT bool writeEntireResource(smtk::mesh::ResourcePtr resource);
SMTKCORE_EXPORT bool writeDomain(smtk::mesh::ResourcePtr resource);
SMTKCORE_EXPORT bool writeDirichlet(smtk::mesh::ResourcePtr resource);
SMTKCORE_EXPORT bool writeNeumann(smtk::mesh::ResourcePtr resource);
} // namespace io
} // namespace smtk

#endif
