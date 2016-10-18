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

namespace smtk {
  namespace io {

ImportMesh::ImportMesh()
{
}

ImportMesh::~ImportMesh()
{
}

std::vector<smtk::io::mesh::MeshIOPtr>& ImportMesh::SupportedIOTypes()
{
  static std::vector<smtk::io::mesh::MeshIOPtr> supportedIOTypes;
  if (supportedIOTypes.empty())
    {
    supportedIOTypes.push_back( mesh::MeshIOPtr( new mesh::MeshIOXMS() ) );
    supportedIOTypes.push_back( mesh::MeshIOPtr( new mesh::MeshIOMoab() ) );
    }
  return supportedIOTypes;
}

smtk::mesh::CollectionPtr ImportMesh::operator() (
  const std::string& filePath,
  smtk::mesh::ManagerPtr manager,
  std::string domainPropertyName) const
{
  // Grab the file extension
  std::string ext = boost::filesystem::extension(filePath);
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

  smtk::mesh::CollectionPtr collection;

  // Search for an appropriate importer
  for (auto&& importer : smtk::io::ImportMesh::SupportedIOTypes())
    {
    for (auto&& format : importer->FileFormats())
      {
      if ( format.CanImport() && std::find(format.Extensions.begin(),
                                           format.Extensions.end(), ext) !=
           format.Extensions.end() )
        {
        // import the collection
        collection = importer->importMesh(filePath, manager,
                                          domainPropertyName);
        break;
        }
      }
    }

  return collection;
}

bool ImportMesh::operator() (const std::string& filePath,
                             smtk::mesh::CollectionPtr collection,
                             std::string domainPropertyName) const
{
  // Grab the file extension
  std::string ext = boost::filesystem::extension(filePath);
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

  // Search for an appropriate importer
  for (auto&& importer : smtk::io::ImportMesh::SupportedIOTypes())
    {
    for (auto&& format : importer->FileFormats())
      {
      if ( format.CanImport() && std::find(format.Extensions.begin(),
                                           format.Extensions.end(), ext) !=
           format.Extensions.end() )
        {
        // import the collection
        return importer->importMesh(filePath, collection, domainPropertyName);
        }
      }
    }
  return false;
}

smtk::mesh::CollectionPtr importMesh(const std::string& filePath,
                                     smtk::mesh::ManagerPtr manager)
{
  ImportMesh importM;
  return importM( filePath, manager, std::string() );
}

smtk::mesh::CollectionPtr importMesh(const std::string& filePath,
                                     smtk::mesh::ManagerPtr manager,
                                     const std::string& domainPropertyName)
{
  ImportMesh importM;
  return importM( filePath, manager, domainPropertyName );
}

bool importMesh(const std::string& filePath,
                smtk::mesh::CollectionPtr collection)
{
  ImportMesh importM;
  return importM( filePath, collection, std::string() );
}

bool importMesh(const std::string& filePath,
                smtk::mesh::CollectionPtr collection,
                const std::string& domainPropertyName)
{
  ImportMesh importM;
  return importM( filePath, collection, domainPropertyName );
}

}
}
