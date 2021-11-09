//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_io_ExportMesh_h
#define smtk_io_ExportMesh_h

#include "smtk/CoreExports.h" // For SMTKCORE_EXPORT macro.
#include "smtk/PublicPointerDefs.h"

#include "smtk/io/mesh/MeshIO.h"

#include <string>
#include <vector>

namespace smtk
{
namespace io
{

/**\brief Export an entire SMTK mesh resource from a file, or just sub-sections
  *
  */
class SMTKCORE_EXPORT ExportMesh
{
public:
  ExportMesh();
  ~ExportMesh();

  ExportMesh& operator=(const ExportMesh&) = delete;
  ExportMesh(const ExportMesh&) = delete;

  static std::vector<smtk::io::mesh::MeshIOPtr>& SupportedIOTypes();

  bool operator()(const std::string& filePath, smtk::mesh::ResourcePtr meshResource) const;
  bool operator()(
    const std::string& filePath,
    smtk::mesh::ResourcePtr meshResource,
    smtk::model::ResourcePtr resource,
    const std::string& modelPropertyName) const;
};

SMTKCORE_EXPORT
bool exportMesh(const std::string& filePath, smtk::mesh::ResourcePtr meshResource);
SMTKCORE_EXPORT
bool exportMesh(
  const std::string& filePath,
  smtk::mesh::ResourcePtr meshResource,
  smtk::model::ResourcePtr resource,
  const std::string& modelPropertyName);
} // namespace io
} // namespace smtk

#endif
