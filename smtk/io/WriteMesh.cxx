//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/io/WriteMesh.h"

#include "smtk/io/mesh/MeshIOMoab.h"
#include "smtk/io/mesh/MeshIOXMS.h"

#include "smtk/common/CompilerInformation.h"

#include "smtk/mesh/core/Resource.h"

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

WriteMesh::WriteMesh() = default;

WriteMesh::~WriteMesh() = default;

std::vector<smtk::io::mesh::MeshIOPtr>& WriteMesh::SupportedIOTypes()
{
  static std::vector<smtk::io::mesh::MeshIOPtr> supportedIOTypes;
  if (supportedIOTypes.empty())
  {
    supportedIOTypes.push_back(mesh::MeshIOPtr(new mesh::MeshIOXMS()));
    supportedIOTypes.push_back(mesh::MeshIOPtr(new mesh::MeshIOMoab()));
  }
  return supportedIOTypes;
}

bool WriteMesh::operator()(
  const std::string& filePath,
  smtk::mesh::ResourcePtr resource,
  mesh::Subset subset) const
{
  // Grab the file extension
  boost::filesystem::path fpath(filePath);
  std::string ext = fpath.extension().string();
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

  // Search for an appropriate writer
  for (auto&& writer : smtk::io::WriteMesh::SupportedIOTypes())
  {
    for (auto&& format : writer->FileFormats())
    {
      if (
        format.CanWrite() &&
        std::find(format.Extensions.begin(), format.Extensions.end(), ext) !=
          format.Extensions.end())
      {
        // write the resource
        return writer->write(filePath, resource, subset);
      }
    }
  }
  return false;
}

bool WriteMesh::operator()(smtk::mesh::ResourcePtr resource, mesh::Subset subset) const
{
  // Grab the file extension
  boost::filesystem::path fpath(resource->writeLocation().absolutePath());
  std::string ext = fpath.extension().string();
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

  // Search for an appropriate writer
  for (auto&& writer : smtk::io::WriteMesh::SupportedIOTypes())
  {
    for (auto&& format : writer->FileFormats())
    {
      if (
        format.CanWrite() &&
        std::find(format.Extensions.begin(), format.Extensions.end(), ext) !=
          format.Extensions.end())
      {
        // write the resource
        return writer->write(resource, subset);
      }
    }
  }
  return false;
}

bool writeMesh(const std::string& filePath, smtk::mesh::ResourcePtr resource, mesh::Subset subset)
{
  WriteMesh write;
  return write(filePath, resource, subset);
}
bool writeEntireResource(const std::string& filePath, smtk::mesh::ResourcePtr resource)
{
  return smtk::io::writeMesh(filePath, resource, mesh::Subset::EntireResource);
}
bool writeDomain(const std::string& filePath, smtk::mesh::ResourcePtr resource)
{
  return smtk::io::writeMesh(filePath, resource, mesh::Subset::OnlyDomain);
}
bool writeDirichlet(const std::string& filePath, smtk::mesh::ResourcePtr resource)
{
  return smtk::io::writeMesh(filePath, resource, mesh::Subset::OnlyDirichlet);
}
bool writeNeumann(const std::string& filePath, smtk::mesh::ResourcePtr resource)
{
  return smtk::io::writeMesh(filePath, resource, mesh::Subset::OnlyNeumann);
}

bool writeMesh(smtk::mesh::ResourcePtr resource, mesh::Subset subset)
{
  WriteMesh write;
  return write(resource, subset);
}
bool writeEntireResource(smtk::mesh::ResourcePtr resource)
{
  WriteMesh write;
  return smtk::io::writeMesh(resource, mesh::Subset::EntireResource);
}
bool writeDomain(smtk::mesh::ResourcePtr resource)
{
  WriteMesh write;
  return smtk::io::writeMesh(resource, mesh::Subset::OnlyDomain);
}
bool writeDirichlet(smtk::mesh::ResourcePtr resource)
{
  WriteMesh write;
  return smtk::io::writeMesh(resource, mesh::Subset::OnlyDirichlet);
}
bool writeNeumann(smtk::mesh::ResourcePtr resource)
{
  WriteMesh write;
  return smtk::io::writeMesh(resource, mesh::Subset::OnlyNeumann);
}
} // namespace io
} // namespace smtk
