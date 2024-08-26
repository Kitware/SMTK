//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/io/ExportMesh.h"

#include "smtk/io/mesh/MeshIOMoab.h"
#include "smtk/io/mesh/MeshIOXMS.h"

#include "smtk/common/CompilerInformation.h"

#include <algorithm>

SMTK_THIRDPARTY_PRE_INCLUDE
#include "boost/filesystem.hpp"
#include "boost/system/error_code.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

using namespace boost::filesystem;

namespace smtk
{
namespace io
{

ExportMesh::ExportMesh() = default;

ExportMesh::~ExportMesh() = default;

std::vector<smtk::io::mesh::MeshIOPtr>& ExportMesh::SupportedIOTypes()
{
  static std::vector<smtk::io::mesh::MeshIOPtr> supportedIOTypes;
  if (supportedIOTypes.empty())
  {
    supportedIOTypes.push_back(mesh::MeshIOPtr(new mesh::MeshIOXMS()));
    supportedIOTypes.push_back(mesh::MeshIOPtr(new mesh::MeshIOMoab()));
  }
  return supportedIOTypes;
}

bool ExportMesh::operator()(const std::string& filePath, smtk::mesh::ResourcePtr meshResource) const
{
  // Grab the file extension
  boost::filesystem::path fpath(filePath);
  std::string ext = fpath.extension().string();
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

  // Search for an appropriate exporter
  for (auto&& exporter : smtk::io::ExportMesh::SupportedIOTypes())
  {
    for (auto&& format : exporter->FileFormats())
    {
      if (
        format.CanExport() &&
        std::find(format.Extensions.begin(), format.Extensions.end(), ext) !=
          format.Extensions.end())
      {
        // export the mesh resource
        return exporter->exportMesh(filePath, meshResource);
      }
    }
  }
  return false;
}

bool ExportMesh::operator()(
  const std::string& filePath,
  smtk::mesh::ResourcePtr meshResource,
  smtk::model::ResourcePtr resource,
  const std::string& modelPropertyName) const
{
  // Grab the file extension
  boost::filesystem::path fpath(filePath);
  std::string ext = fpath.extension().string();
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

  // Search for an appropriate exporter
  for (auto&& exporter : smtk::io::ExportMesh::SupportedIOTypes())
  {
    for (auto&& format : exporter->FileFormats())
    {
      if (
        format.CanExport() &&
        std::find(format.Extensions.begin(), format.Extensions.end(), ext) !=
          format.Extensions.end())
      {
        // export the mesh resource
        return exporter->exportMesh(filePath, meshResource, resource, modelPropertyName);
      }
    }
  }
  return false;
}

bool exportMesh(const std::string& filePath, smtk::mesh::ResourcePtr meshResource)
{
  ExportMesh exportM;
  return exportM(filePath, meshResource);
}

bool exportMesh(
  const std::string& filePath,
  smtk::mesh::ResourcePtr meshResource,
  smtk::model::ResourcePtr resource,
  const std::string& modelPropertyName)
{
  ExportMesh exportM;
  return exportM(filePath, meshResource, resource, modelPropertyName);
}
} // namespace io
} // namespace smtk
