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

#include "smtk/mesh/Collection.h"

#include <algorithm>

SMTK_THIRDPARTY_PRE_INCLUDE
#include "boost/filesystem.hpp"
#include "boost/system/error_code.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

using namespace boost::filesystem;

namespace smtk {
  namespace io {

WriteMesh::WriteMesh()
{
  this->IO.push_back( smtk::io::mesh::MeshIOPtr( new mesh::MeshIOXMS() ) );
  this->IO.push_back( smtk::io::mesh::MeshIOPtr( new mesh::MeshIOMoab() ) );
}

WriteMesh::~WriteMesh()
{
}

bool WriteMesh::operator() (const std::string& filePath,
                            smtk::mesh::CollectionPtr collection,
                            mesh::Subset subset) const
{
  // Grab the file extension
  std::string ext = boost::filesystem::extension(filePath);
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

  // Search for an appropriate writer
  for (auto&& writer : this->IO)
    {
    for (auto&& format : writer->FileFormats())
      {
      if ( format.CanWrite() && std::find(format.Extensions.begin(),
                                          format.Extensions.end(), ext) !=
           format.Extensions.end() )
        {
        // write the collection
        return writer->write(filePath, collection, subset);
        }
      }
    }
  return false;
}

bool WriteMesh::operator() (smtk::mesh::CollectionPtr collection,
                            mesh::Subset subset) const
{
  // Grab the file extension
  std::string ext = boost::filesystem::extension(
    collection->writeLocation().absolutePath());
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

  // Search for an appropriate writer
  for (auto&& writer : this->IO)
    {
    for (auto&& format : writer->FileFormats())
      {
      if ( format.CanWrite() && std::find(format.Extensions.begin(),
                                          format.Extensions.end(), ext) !=
           format.Extensions.end() )
      {
      // write the collection
      return writer->write(collection, subset);
      }
    }
    }
  return false;
}

bool writeMesh( const std::string& filePath,
                smtk::mesh::CollectionPtr collection,
                mesh::Subset subset )
{
  WriteMesh write;
  return write( filePath, collection, subset );
}

bool writeMesh( smtk::mesh::CollectionPtr collection,
                mesh::Subset subset )
{
  WriteMesh write;
  return write( collection, subset );
}

}
}
