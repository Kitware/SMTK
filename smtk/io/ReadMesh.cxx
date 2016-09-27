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

#include "smtk/mesh/Collection.h"

#include <algorithm>

SMTK_THIRDPARTY_PRE_INCLUDE
#include "boost/filesystem.hpp"
#include "boost/system/error_code.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

using namespace boost::filesystem;

namespace smtk {
  namespace io {

ReadMesh::ReadMesh()
{
  this->IO.push_back(new mesh::MeshIOXMS());
  this->IO.push_back(new mesh::MeshIOMoab());
}

ReadMesh::~ReadMesh()
{
  for (auto&& reader : this->IO)
    {
    delete reader;
    }
}

smtk::mesh::CollectionPtr
ReadMesh::operator() (const std::string& filePath,
                      smtk::mesh::ManagerPtr manager,
                      mesh::Subset subset) const
{
  // Grab the file extension
  std::string ext = boost::filesystem::extension(filePath);
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

  smtk::mesh::CollectionPtr collection = nullptr;

  // Search for an appropriate reader
  for (auto&& reader : this->IO)
    {
    for (auto&& format : reader->FileFormats())
      {
      if ( format.CanRead() && std::find(format.Extensions.begin(),
                                         format.Extensions.end(), ext) !=
           format.Extensions.end() )
        {
        // read the collection
        collection = reader->read(filePath, manager, subset);
        break;
        }
      }
    }

  if (!collection)
    {
    collection = smtk::mesh::Collection::create();
    collection->readLocation(filePath);
    }

  return collection;
}

bool ReadMesh::operator() (const std::string& filePath,
                           smtk::mesh::CollectionPtr collection,
                           mesh::Subset subset) const
{
  // Grab the file extension
  std::string ext = boost::filesystem::extension(filePath);
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

  // Search for an appropriate reader
  for (auto&& reader : this->IO)
    {
    for (auto&& format : reader->FileFormats())
      {
      if ( format.CanRead() && std::find(format.Extensions.begin(),
                                         format.Extensions.end(), ext) !=
           format.Extensions.end() )
        {
        // read the collection
        return reader->read(filePath, collection, subset);
        }
      }
    }
  return false;
}


}
}
