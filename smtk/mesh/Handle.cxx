//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/mesh/Handle.h"
#include "cJSON.h"

#include <cstdint>
#include <iostream>
#include <sstream>


namespace smtk {
namespace mesh {

namespace detail
{

#define MOAB_TYPE_WIDTH 4
#define MOAB_ID_WIDTH (8*sizeof(smtk::mesh::Handle)-MOAB_TYPE_WIDTH)
#define MOAB_TYPE_MASK ((smtk::mesh::Handle)0xF << MOAB_ID_WIDTH)
#define MOAB_ID_MASK (~MOAB_TYPE_MASK)

  inline ::moab::EntityID to_id(smtk::mesh::Handle handle)
  {
  return (handle & MOAB_ID_MASK);
  }

  inline smtk::mesh::Handle to_handle(::moab::EntityType type,
                                      ::moab::EntityID id)
  {
  if (type > ::moab::MBMAXTYPE)
    {
    return smtk::mesh::Handle();  //<You've got to return something.  What do you return?
    }
  return (((smtk::mesh::Handle)type) << MOAB_ID_WIDTH)|id;
  }

#undef MOAB_TYPE_WIDTH
#undef MOAB_ID_WIDTH
#undef MOAB_TYPE_MASK
#undef MOAB_ID_MASK

  //presumes that all values in the range are withing the same entity type
  cJSON* subset_to_json_array(const smtk::mesh::HandleRange& range)
  {
  cJSON* array = cJSON_CreateArray();
  //iterate the range finding
  typedef smtk::mesh::HandleRange::const_pair_iterator const_pair_iterator;

  for(const_pair_iterator i=range.begin();
      i != range.end();
      ++i)
    {
    ::moab::EntityID start = to_id(i->first);
    ::moab::EntityID end = to_id(i->second);

    cJSON_AddItemToArray(array, cJSON_CreateNumber(static_cast<double>(start)));
    cJSON_AddItemToArray(array, cJSON_CreateNumber(static_cast<double>(end)));
    }

  return array;
  }

  //presumes that all values in the range are withing the same entity type
  smtk::mesh::HandleRange subset_from_json_array(int type,
                                                 cJSON* array)
  {
  smtk::mesh::HandleRange result;
  ::moab::EntityType et = static_cast< ::moab::EntityType >(type);
  cJSON* n = array->child;
  while(n)
    {
    ::moab::EntityID start =
     to_handle(et, static_cast<::moab::EntityID>(n->valuedouble));
    ::moab::EntityID end =
     to_handle(et, static_cast<::moab::EntityID>(n->next->valuedouble));
    result.insert(start,end);

    //since we are reading two nodes at a time
    if(n->next)
      { n = n->next->next; }
    else
      { n = NULL; }
    }

  return result;
  }
}

//convert a handle range to a json formatted node
cJSON* to_json(const smtk::mesh::HandleRange& range)
{
  cJSON* json_dict = cJSON_CreateObject();

  //first we subset by type
  std::stringstream buffer;
  std::string typeAsString;
  for(::moab::EntityType i= ::moab::MBVERTEX;
      i != ::moab::MBMAXTYPE;
      ++i)
  {
    smtk::mesh::HandleRange subset = range.subset_by_type(i);
    if(subset.empty())
      {
      continue;
      }

    //build the name
    buffer << i;
    buffer >> typeAsString;
    buffer.clear();

    cJSON* jarray = detail::subset_to_json_array(subset);
    cJSON_AddItemToObject(json_dict, typeAsString.c_str(), jarray);
  }

  return json_dict;
}

//convert json formatted string to a handle range
smtk::mesh::HandleRange from_json(cJSON* json)
{
  smtk::mesh::HandleRange result;
  if(!json)
  {
    return result;
  }

  std::stringstream buffer;
  //iterate the children
  for (cJSON* arr = json->child; arr; arr = arr->next)
    {
    if (arr->type == cJSON_Array)
      {
      //extract the name
      int type;
      buffer << std::string(arr->string);
      buffer >> type;
      buffer.clear();

      smtk::mesh::HandleRange subset = detail::subset_from_json_array(type,
                                                                      arr);
      result.merge(subset);
      }
    }
  return result;
}

}
}
