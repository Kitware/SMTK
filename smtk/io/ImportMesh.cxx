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

SMTK_THIRDPARTY_PRE_INCLUDE
#include "boost/filesystem.hpp"
#include "boost/system/error_code.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

using namespace boost::filesystem;

namespace smtk {
  namespace io {

ImportMesh::ImportMesh()
{
  this->IO.push_back( smtk::io::mesh::MeshIOPtr( new mesh::MeshIOXMS() ) );
  this->IO.push_back( smtk::io::mesh::MeshIOPtr( new mesh::MeshIOMoab() ) );
}

ImportMesh::~ImportMesh()
{
}

smtk::mesh::CollectionPtr ImportMesh::operator() (const std::string& filePath,
                                                  smtk::mesh::ManagerPtr manager) const
{
  // Grab the file extension
  std::string ext = boost::filesystem::extension(filePath);
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

  smtk::mesh::CollectionPtr collection;

  // Search for an appropriate importer
  for (auto&& importer : this->IO)
    {
    for (auto&& format : importer->FileFormats())
      {
      if ( format.CanImport() && std::find(format.Extensions.begin(),
                                           format.Extensions.end(), ext) !=
           format.Extensions.end() )
        {
        // import the collection
        collection = importer->importMesh(filePath, manager);
        break;
        }
      }
    }

  return collection;
}

bool ImportMesh::operator() (const std::string& filePath,
                             smtk::mesh::CollectionPtr collection) const
{
  // Grab the file extension
  std::string ext = boost::filesystem::extension(filePath);
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

  // Search for an appropriate importer
  for (auto&& importer : this->IO)
    {
    for (auto&& format : importer->FileFormats())
      {
      if ( format.CanImport() && std::find(format.Extensions.begin(),
                                           format.Extensions.end(), ext) !=
           format.Extensions.end() )
        {
        // import the collection
        return importer->importMesh(filePath, collection);
        }
      }
    }
  return false;
}

smtk::mesh::CollectionPtr importMesh(const std::string& filePath,
                                     smtk::mesh::ManagerPtr manager)
{
  ImportMesh importM;
  return importM( filePath, manager );
}

bool importMesh(const std::string& filePath,
                smtk::mesh::CollectionPtr collection)
{
{
  ImportMesh importM;
  return importM( filePath, collection );
}
}

}
}
