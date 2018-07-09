//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/json/jsonEntity.h"

#include "smtk/common/json/jsonUUID.h"

#include "smtk/model/Entity.h"

#include "nlohmann/json.hpp"

#include <exception>
#include <string>

namespace smtk
{
namespace model
{
// Define how Entity records are serialized.
using BitFlags = smtk::model::BitFlags;
using json = nlohmann::json;
void to_json(json& j, const smtk::model::EntityPtr& ent)
{
  j = { { "id", ent->id() }, { "d", ent->dimensionBits() },
    { "e", smtk::model::Entity::flagToSpecifierString(ent->entityFlags()) },
    { "r", ent->relations() } };
}

void from_json(const json& j, smtk::model::EntityPtr& ent)
{
  if (j.is_null())
  {
    ent = smtk::model::Entity::create();
  }
  try
  {
    BitFlags entFlags = smtk::model::Entity::specifierStringToFlag(j.at("e"));
    ent = smtk::model::Entity::create(j.at("id"), entFlags);
    ent->relations() = j.at("r").get<smtk::common::UUIDArray>();
  }
  catch (std::exception&)
  {
  }
}

// Define how maps of Entity records are serialized.
using BitFlags = smtk::model::BitFlags;
using UUID = smtk::common::UUID;
using UUIDArray = smtk::common::UUIDArray;
using Entity = smtk::model::Entity;
using EntityPtr = smtk::model::EntityPtr;
using UUIDsToEntities = smtk::model::UUIDsToEntities;

void to_json(json& j, const UUIDsToEntities& emap)
{
  j = json::object();
  for (auto epair : emap)
  {
    j[epair.first.toString()] = { { "d", epair.second->dimensionBits() },
      { "e", smtk::model::Entity::flagToSpecifierString(epair.second->entityFlags()) },
      { "r", epair.second->relations() } };
  };
}

void from_json(const json& j, UUIDsToEntities& emap)
{
  for (json::const_iterator it = j.begin(); it != j.end(); ++it)
  {
    try
    {
      BitFlags entFlags = smtk::model::Entity::specifierStringToFlag(j.at("e"));
      EntityPtr ent = Entity::create(UUID(it.key()), entFlags);
      ent->relations() = it.value().at("r").get<UUIDArray>();
      emap[ent->id()] = ent;
    }
    catch (std::exception&)
    {
    }
  }
}
}
}
