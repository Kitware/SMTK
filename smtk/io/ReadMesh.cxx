//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/io/ReadMesh.h"

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

ReadMesh::ReadMesh() = default;

ReadMesh::~ReadMesh() = default;

std::vector<smtk::io::mesh::MeshIOPtr>& ReadMesh::SupportedIOTypes()
{
  static std::vector<smtk::io::mesh::MeshIOPtr> supportedIOTypes;
  if (supportedIOTypes.empty())
  {
    supportedIOTypes.push_back(mesh::MeshIOPtr(new mesh::MeshIOXMS()));
    supportedIOTypes.push_back(mesh::MeshIOPtr(new mesh::MeshIOMoab()));
  }
  return supportedIOTypes;
}

bool ReadMesh::ExtensionIsSupported(const std::string& ext)
{
  for (auto& reader : smtk::io::ReadMesh::SupportedIOTypes())
  {
    for (const auto& format : reader->FileFormats())
    {
      if (
        format.CanRead() &&
        std::find(format.Extensions.begin(), format.Extensions.end(), ext) !=
          format.Extensions.end())
      {
        return true;
      }
    }
  }

  return false;
}

smtk::mesh::ResourcePtr ReadMesh::operator()(
  const std::string& filePath,
  const smtk::mesh::InterfacePtr& interface,
  mesh::Subset subset) const
{
  // Grab the file extension
  boost::filesystem::path fpath(filePath);
  std::string ext = fpath.extension().string();
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

  smtk::mesh::ResourcePtr resource = nullptr;

  // Search for an appropriate reader
  for (auto&& reader : smtk::io::ReadMesh::SupportedIOTypes())
  {
    for (auto&& format : reader->FileFormats())
    {
      if (
        format.CanRead() &&
        std::find(format.Extensions.begin(), format.Extensions.end(), ext) !=
          format.Extensions.end())
      {
        // read the resource
        resource = reader->read(filePath, interface, subset);
        break;
      }
    }
  }

  if (!resource)
  {
    // create an invalid colection (i.e. one with an invalid id)
    resource = smtk::mesh::Resource::create();
    resource->setId(smtk::common::UUID::null());
    resource->readLocation(filePath);
  }
  else
  {
    resource->readLocation(filePath);
  }

  return resource;
}

bool ReadMesh::operator()(
  const std::string& filePath,
  smtk::mesh::ResourcePtr resource,
  mesh::Subset subset) const
{
  // Grab the file extension
  boost::filesystem::path fpath(filePath);
  std::string ext = fpath.extension().string();
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

  // Search for an appropriate reader
  for (auto&& reader : smtk::io::ReadMesh::SupportedIOTypes())
  {
    for (auto&& format : reader->FileFormats())
    {
      if (
        format.CanRead() &&
        std::find(format.Extensions.begin(), format.Extensions.end(), ext) !=
          format.Extensions.end())
      {
        // read the resource
        return reader->read(filePath, resource, subset);
      }
    }
  }
  return false;
}

smtk::mesh::ResourcePtr readMesh(
  const std::string& filePath,
  const smtk::mesh::InterfacePtr& interface,
  mesh::Subset subset)
{
  ReadMesh read;
  return read(filePath, interface, subset);
}
smtk::mesh::ResourcePtr readEntireResource(
  const std::string& filePath,
  const smtk::mesh::InterfacePtr& interface)
{
  return smtk::io::readMesh(filePath, interface, mesh::Subset::EntireResource);
}
smtk::mesh::ResourcePtr readDomain(
  const std::string& filePath,
  const smtk::mesh::InterfacePtr& interface)
{
  return smtk::io::readMesh(filePath, interface, mesh::Subset::OnlyDomain);
}
smtk::mesh::ResourcePtr readDirichlet(
  const std::string& filePath,
  const smtk::mesh::InterfacePtr& interface)
{
  return smtk::io::readMesh(filePath, interface, mesh::Subset::OnlyDirichlet);
}
smtk::mesh::ResourcePtr readNeumann(
  const std::string& filePath,
  const smtk::mesh::InterfacePtr& interface)
{
  return smtk::io::readMesh(filePath, interface, mesh::Subset::OnlyNeumann);
}

bool readMesh(const std::string& filePath, smtk::mesh::ResourcePtr resource, mesh::Subset subset)
{
  ReadMesh read;
  return read(filePath, resource, subset);
}
bool readEntireResource(const std::string& filePath, smtk::mesh::ResourcePtr resource)
{
  return smtk::io::readMesh(filePath, resource, mesh::Subset::EntireResource);
}
bool readDomain(const std::string& filePath, smtk::mesh::ResourcePtr resource)
{
  return smtk::io::readMesh(filePath, resource, mesh::Subset::OnlyDomain);
}
bool readDirichlet(const std::string& filePath, smtk::mesh::ResourcePtr resource)
{
  return smtk::io::readMesh(filePath, resource, mesh::Subset::OnlyDirichlet);
}
bool readNeumann(const std::string& filePath, smtk::mesh::ResourcePtr resource)
{
  return smtk::io::readMesh(filePath, resource, mesh::Subset::OnlyNeumann);
}
} // namespace io
} // namespace smtk
