//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/io/ImportMesh.h"

#include "smtk/io/mesh/MeshIOMoab.h"
#include "smtk/io/mesh/MeshIOXMS.h"

#include "smtk/common/CompilerInformation.h"

#include <algorithm>
#include <iostream>

SMTK_THIRDPARTY_PRE_INCLUDE
#include "boost/filesystem.hpp"
#include "boost/system/error_code.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

using namespace boost::filesystem;

namespace smtk
{
namespace io
{

ImportMesh::ImportMesh() = default;

ImportMesh::~ImportMesh() = default;

std::vector<smtk::io::mesh::MeshIOPtr>& ImportMesh::SupportedIOTypes()
{
  static std::vector<smtk::io::mesh::MeshIOPtr> supportedIOTypes;
  if (supportedIOTypes.empty())
  {
    supportedIOTypes.push_back(mesh::MeshIOPtr(new mesh::MeshIOXMS()));
    supportedIOTypes.push_back(mesh::MeshIOPtr(new mesh::MeshIOMoab()));
  }
  return supportedIOTypes;
}

bool ImportMesh::ExtensionIsSupported(const std::string& ext)
{
  for (auto& importer : smtk::io::ImportMesh::SupportedIOTypes())
  {
    for (const auto& format : importer->FileFormats())
    {
      if (
        format.CanImport() &&
        std::find(format.Extensions.begin(), format.Extensions.end(), ext) !=
          format.Extensions.end())
      {
        return true;
      }
    }
  }

  return false;
}

smtk::io::mesh::Format ImportMesh::fileFormat(const std::string& filePath)
{
  // Grab the file extension
  boost::filesystem::path fpath(filePath);
  std::string ext = fpath.extension().string();
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

  for (auto& importer : smtk::io::ImportMesh::SupportedIOTypes())
  {
    for (const auto& format : importer->FileFormats())
    {
      if (
        format.CanImport() &&
        std::find(format.Extensions.begin(), format.Extensions.end(), ext) !=
          format.Extensions.end())
      {
        return format;
      }
    }
  }

  return smtk::io::mesh::Format();
}

smtk::mesh::ResourcePtr ImportMesh::operator()(
  const std::string& filePath,
  const smtk::mesh::InterfacePtr& interface,
  std::string domainPropertyName) const
{
  // Grab the file extension
  boost::filesystem::path fpath(filePath);
  std::string ext = fpath.extension().string();
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

  // Search for an appropriate importer
  for (auto&& importer : smtk::io::ImportMesh::SupportedIOTypes())
  {
    for (auto&& format : importer->FileFormats())
    {
      if (
        format.CanImport() &&
        std::find(format.Extensions.begin(), format.Extensions.end(), ext) !=
          format.Extensions.end())
      {
        // import the resource
        return importer->importMesh(filePath, interface, domainPropertyName);
      }
    }
  }

  return smtk::mesh::ResourcePtr();
}

bool ImportMesh::operator()(
  const std::string& filePath,
  smtk::mesh::ResourcePtr resource,
  std::string domainPropertyName) const
{
  // Grab the file extension
  boost::filesystem::path fpath(filePath);
  std::string ext = fpath.extension().string();
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

  // Search for an appropriate importer
  for (auto&& importer : smtk::io::ImportMesh::SupportedIOTypes())
  {
    for (auto&& format : importer->FileFormats())
    {
      if (
        format.CanImport() &&
        std::find(format.Extensions.begin(), format.Extensions.end(), ext) !=
          format.Extensions.end())
      {
        // import the resource
        return importer->importMesh(filePath, resource, domainPropertyName);
      }
    }
  }
  return false;
}

smtk::mesh::ResourcePtr importMesh(
  const std::string& filePath,
  const smtk::mesh::InterfacePtr& interface)
{
  ImportMesh importM;
  return importM(filePath, interface, std::string());
}

smtk::mesh::ResourcePtr importMesh(
  const std::string& filePath,
  const smtk::mesh::InterfacePtr& interface,
  const std::string& domainPropertyName)
{
  ImportMesh importM;
  return importM(filePath, interface, domainPropertyName);
}

bool importMesh(const std::string& filePath, smtk::mesh::ResourcePtr resource)
{
  ImportMesh importM;
  return importM(filePath, resource, std::string());
}

bool importMesh(
  const std::string& filePath,
  smtk::mesh::ResourcePtr resource,
  const std::string& domainPropertyName)
{
  ImportMesh importM;
  return importM(filePath, resource, domainPropertyName);
}

smtk::io::mesh::Format meshFileFormat(const std::string& filePath)
{
  ImportMesh importM;
  return importM.fileFormat(filePath);
}
} // namespace io
} // namespace smtk
