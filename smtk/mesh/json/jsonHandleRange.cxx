//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/mesh/json/jsonHandleRange.h"

// Define how resources are serialized.
namespace
{
using json = nlohmann::json;

#define MOAB_TYPE_WIDTH 4
#define MOAB_ID_WIDTH (8 * sizeof(smtk::mesh::Handle) - MOAB_TYPE_WIDTH)
#define MOAB_TYPE_MASK ((smtk::mesh::Handle)0xF << MOAB_ID_WIDTH)
#define MOAB_ID_MASK (~MOAB_TYPE_MASK)

inline ::moab::EntityID to_id(smtk::mesh::Handle handle)
{
  return (handle & MOAB_ID_MASK);
}

inline smtk::mesh::Handle to_handle(::moab::EntityType type, ::moab::EntityID id)
{
  if (type > ::moab::MBMAXTYPE)
  {
    return smtk::mesh::Handle(); //<You've got to return something.  What do you return?
  }
  return (((smtk::mesh::Handle)type) << MOAB_ID_WIDTH) | id;
}

#undef MOAB_TYPE_WIDTH
#undef MOAB_ID_WIDTH
#undef MOAB_TYPE_MASK
#undef MOAB_ID_MASK

//presumes that all values in the range are withing the same entity type
json subset_to_json_array(const smtk::mesh::HandleRange& range)
{
  json j = json::array();

  //iterate the range finding
  typedef smtk::mesh::HandleRange::const_pair_iterator const_pair_iterator;

  for (const_pair_iterator i = range.begin(); i != range.end(); ++i)
  {
    ::moab::EntityID start = to_id(i->first);
    ::moab::EntityID end = to_id(i->second);

    j.push_back(start);
    j.push_back(end);
  }
  return j;
}

//presumes that all values in the range are withing the same entity type
smtk::mesh::HandleRange subset_from_json_array(int type, const json& array)
{
  smtk::mesh::HandleRange result;
  ::moab::EntityType et = static_cast< ::moab::EntityType>(type);
  for (json::const_iterator it = array.begin(); it != array.end(); it += 2)
  {
    ::moab::EntityID start = to_handle(et, *it);
    ::moab::EntityID end = to_handle(et, *(std::next(it)));
    result.insert(result.end(), start, end);
  }

  return result;
}
}

namespace smtk
{
namespace mesh
{
void to_json(json& j, const smtk::mesh::HandleRange& handleRange)
{
  //first we subset by type
  std::stringstream buffer;
  std::string typeAsString;
  for (::moab::EntityType i = ::moab::MBVERTEX; i != ::moab::MBMAXTYPE; ++i)
  {
    smtk::mesh::HandleRange subset = handleRange.subset_by_type(i);
    if (subset.empty())
    {
      continue;
    }

    j[std::to_string(i)] = subset_to_json_array(subset);
  }
}

void from_json(const json& j, smtk::mesh::HandleRange& handleRange)
{
  if (j.is_null())
  {
    return;
  }

  //iterate the children
  for (json::const_iterator it = j.begin(); it != j.end(); ++it)
  {
    //extract the name
    int type = std::stoi(it.key());

    smtk::mesh::HandleRange subset = subset_from_json_array(type, *it);
    handleRange.merge(subset);
  }
}
}
}
