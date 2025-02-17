//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/ResourceItem.h"

#include "smtk/common/Paths.h"

#include "smtk/io/ReadMesh.h"

#include "smtk/mesh/core/Resource.h"

#include "smtk/mesh/operators/ReadResource.h"
#include "smtk/mesh/operators/WriteResource.h"

#include "smtk/mesh/testing/cxx/helpers.h"

//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
using namespace boost::filesystem;

namespace
{

//SMTK_DATA_DIR is a define setup by cmake
std::string data_root = SMTK_DATA_DIR;
std::string write_root = SMTK_SCRATCH_DIR;

void cleanup(const std::string& file_path)
{
  //first verify the file exists
  ::boost::filesystem::path path(file_path);
  if (::boost::filesystem::is_regular_file(path))
  {
    //remove the file_path if it exists.
    ::boost::filesystem::remove(path);
  }
}
} // namespace

int UnitTestReadWriteMeshResource(int /*unused*/, char** const /*unused*/)
{
  std::string file_path(data_root);
  file_path += "/mesh/3d/twoassm_out.h5m";

  std::string write_path(write_root);
  write_path += "/" + smtk::common::UUID::random().toString() + ".smtk";

  smtk::io::ReadMesh read;
  smtk::mesh::ResourcePtr mr = smtk::mesh::Resource::create();
  read(file_path, mr);
  test(mr->isValid(), "resource should be valid");
  test(!mr->isModified(), "resource shouldn't be marked as modified");

  mr->setLocation(write_path);

  //write out the mesh.
  auto writeOp = smtk::mesh::WriteResource::create();
  test(writeOp != nullptr, "failed to create write resource operation");
  test(writeOp->parameters() != nullptr, "failed to access write resource operation parameters");
  test(
    writeOp->parameters()->associate(mr),
    "failed to associate mesh resource to write resource operation");
  auto result = writeOp->operate();
  test(
    result->findInt("outcome")->value() ==
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED),
    "write resource operation failed to operate");

  auto readOp = smtk::mesh::ReadResource::create();
  test(readOp != nullptr, "failed to create read resource operation");
  test(readOp->parameters() != nullptr, "failed to access read resource operation parameters");
  test(
    readOp->parameters()->findFile("filename")->setValue(write_path),
    "failed to set file path for read resource operation");
  result = readOp->operate();
  test(
    result->findInt("outcome")->value() ==
      static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED),
    "read resource operation failed to operate");

  //reload the written file and verify the number of meshes are the same as the
  //input mesh
  smtk::mesh::ResourcePtr mr1 = std::dynamic_pointer_cast<smtk::mesh::Resource>(
    result->findResource("resourcesCreated")->value());
  test(mr1 != nullptr, "could not access read resource operation result");

  //remove the files from disk
  std::string meshFilename = smtk::common::Paths::directory(mr->location()) + "/" +
    smtk::common::Paths::stem(mr->location()) + ".h5m";
  cleanup(write_path);
  cleanup(meshFilename);

  //verify the meshes
  test(mr1->isValid(), "resource should be valid");
  test(mr1->name() == mr->name());
  test(mr1->numberOfMeshes() == mr->numberOfMeshes());
  test(mr1->types() == mr->types());

  return 0;
}
