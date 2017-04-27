//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/Collection.h"
#include "smtk/mesh/Handle.h"
#include "smtk/mesh/Manager.h"

#include "smtk/mesh/testing/cxx/helpers.h"

//force to use filesystem version 3
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
using namespace boost::filesystem;

namespace
{

inline smtk::mesh::Handle to_handle(::moab::EntityType type, ::moab::EntityID id)
{
  const std::size_t id_width = (8 * sizeof(smtk::mesh::Handle) - 4);
  if (type > ::moab::MBMAXTYPE)
  {
    return smtk::mesh::Handle();
  }
  return (((smtk::mesh::Handle)type) << id_width) | id;
}

void verify_empty_handle()
{
  smtk::mesh::HandleRange range;

  //convert empty handle to json
  cJSON* json = smtk::mesh::to_json(range);

  //verify that the json has no children, since the range is empty
  test((json->child == NULL), "empty handle range json form should be empty");

  //convert back to a handle
  smtk::mesh::HandleRange result = smtk::mesh::from_json(json);

  test(result == range, "empty handle didn't serialize properly");
}

void verify_meshset_handle()
{
  smtk::mesh::Handle first = to_handle(::moab::MBENTITYSET, 1);
  smtk::mesh::Handle second = to_handle(::moab::MBENTITYSET, 1024);

  //make range have 1024 meshes
  smtk::mesh::HandleRange range;
  range.insert(first, second);

  test(range.size() == 1024, "verify range has proper length");
  test(range.all_of_type(::moab::MBENTITYSET) == true, "verify range is all mesh sets");

  //now verify that it converts to json properly
  cJSON* json = smtk::mesh::to_json(range);

  //verify that the json has no children, since the range is empty
  test((json->child != NULL), "meshset handle json form should have a child node");

  //convert back to a handle
  smtk::mesh::HandleRange result = smtk::mesh::from_json(json);
  test(result == range, "meshset handle didn't serialize properly");
}

void verify_single_cell_type_handle()
{
  smtk::mesh::Handle first = to_handle(::moab::MBHEX, 1);
  smtk::mesh::Handle second = to_handle(::moab::MBHEX, 450);

  //make range have 1024 meshes
  smtk::mesh::HandleRange range;
  range.insert(first, second);

  //now verify that it converts to json properly
  cJSON* json = smtk::mesh::to_json(range);

  //verify that the json has no children, since the range is empty
  test((json->child != NULL), "meshset handle json form should have a child node");

  //convert back to a handle
  smtk::mesh::HandleRange result = smtk::mesh::from_json(json);
  test(result == range, "single cell set handle didn't serialize properly");
}

void verify_mixed_handle()
{
  smtk::mesh::Handle first = to_handle(::moab::MBENTITYSET, 1);
  smtk::mesh::Handle second = to_handle(::moab::MBENTITYSET, 45);

  smtk::mesh::Handle third = to_handle(::moab::MBENTITYSET, 100);
  smtk::mesh::Handle fourth = to_handle(::moab::MBENTITYSET, 450);

  smtk::mesh::Handle fifth = to_handle(::moab::MBHEX, 1);
  smtk::mesh::Handle sixth = to_handle(::moab::MBHEX, 450);

  //make range have 1024 meshes
  smtk::mesh::HandleRange range;
  range.insert(first, second);
  range.insert(third, fourth);
  range.insert(fifth, sixth);

  //now verify that it converts to json properly
  cJSON* json = smtk::mesh::to_json(range);

  //verify that the json has no children, since the range is empty
  test((json->child != NULL), "meshset handle json form should have a child node");

  //convert back to a handle
  smtk::mesh::HandleRange result = smtk::mesh::from_json(json);
  test(result == range, "mixed cell set handle didn't serialize properly");
}

void verify_large_number_of_values_handle()
{
  smtk::mesh::Handle first = to_handle(::moab::MBENTITYSET, 1);
  smtk::mesh::Handle second = to_handle(::moab::MBENTITYSET, 2048);

  smtk::mesh::Handle third = to_handle(::moab::MBHEX, 0);
  smtk::mesh::Handle fourth = to_handle(::moab::MBHEX, 4194303);

  smtk::mesh::Handle fifth = to_handle(::moab::MBHEX, 0);
  smtk::mesh::Handle sixth = to_handle(::moab::MBHEX, 8388607);

  //make range have 1024 meshes
  smtk::mesh::HandleRange range;
  range.insert(first, second);
  range.insert(third, fourth);
  range.insert(fifth, sixth);

  //now verify that it converts to json properly
  cJSON* json = smtk::mesh::to_json(range);

  //verify that the json has no children, since the range is empty
  test((json->child != NULL), "meshset handle json form should have a child node");

  //convert back to a handle
  smtk::mesh::HandleRange result = smtk::mesh::from_json(json);
  test(result == range, "mixed cell set handle didn't serialize properly");
}
}

int UnitTestReadWriteHandles(int, char** const)
{
  verify_empty_handle();

  verify_meshset_handle();
  verify_single_cell_type_handle();
  verify_mixed_handle();

  verify_large_number_of_values_handle();
  return 0;
}
