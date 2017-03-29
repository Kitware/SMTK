//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/model/operators/WriteMesh.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/MeshItem.h"

#include "smtk/io/WriteMesh.h"
#include "smtk/io/mesh/MeshIO.h"

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Manager.h"
#include "smtk/mesh/MeshSet.h"

#include "smtk/model/Manager.h"
#include "smtk/model/Session.h"

#include <sstream>
#include <string>

//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
using namespace boost::filesystem;

namespace
{
void cleanup( const std::string& file_path )
{
  //first verify the file exists
  ::boost::filesystem::path path( file_path );
  if( ::boost::filesystem::is_regular_file( path ) )
    {
    //remove the file_path if it exists.
    ::boost::filesystem::remove( path );
    }
}
}

namespace smtk {
  namespace model {

bool WriteMesh::ableToOperate()
{
  if(!this->ensureSpecification())
    return false;
  smtk::attribute::MeshItem::Ptr meshItem =
    this->specification()->findMesh("mesh");
  return meshItem && meshItem->numberOfValues() > 0;
}

smtk::model::OperatorResult WriteMesh::operateInternal()
{
  std::string outputfile =
    this->specification()->findFile("filename")->value();

  smtk::io::mesh::Subset componentToWrite =
    static_cast<smtk::io::mesh::Subset>( this->specification()->
                                         findInt("write-component")->value() );

  // ableToOperate should have verified that mesh(s) are set
  smtk::attribute::MeshItem::Ptr meshItem =
    this->specification()->findMesh("mesh");

  // for multiple meshes, we suffix the file name root with ascending integers
  std::string root = outputfile.substr(0, outputfile.find_last_of("."));
  std::string ext = outputfile.substr(outputfile.find_last_of("."));
  int index = 0;

  smtk::mesh::MeshSets written;
  std::vector<std::string> generatedFiles;

  for (attribute::MeshItem::const_mesh_it mit = meshItem->begin();
       mit != meshItem->end(); ++mit)
    {
    smtk::mesh::CollectionPtr collection = mit->collection();
    bool fileWriteSuccess = false;

    if(collection)
      {
      if (meshItem->numberOfValues() > 1)
        {
        std::stringstream s; s << root << "_" << index << ext;
        outputfile = s.str();
        }

      smtk::io::WriteMesh write;
      fileWriteSuccess = write(outputfile, collection, componentToWrite);

      if(fileWriteSuccess)
        {
        ++index;
        generatedFiles.push_back(outputfile);
        written.insert(*mit);
        }
      }

    if (fileWriteSuccess == false)
      {
      for (auto&& file : generatedFiles)
        {
        cleanup(file);
        }
      return this->createResult(OPERATION_FAILED);
      }
    }

  return this->createResult(OPERATION_SUCCEEDED);
}

}
}

#include "smtk/model/WriteMesh_xml.h"

smtkImplementsModelOperator(
  SMTKCORE_EXPORT,
  smtk::model::WriteMesh,
  write_mesh,
  "write mesh",
  WriteMesh_xml,
  smtk::model::Session);
