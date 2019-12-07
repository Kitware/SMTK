//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/json/jsonEntityIterator.h"

#include "smtk/model/json/jsonArrangement.h"
#include "smtk/model/json/jsonTessellation.h"

#include "smtk/common/json/jsonUUID.h"

#include "smtk/model/Arrangement.h"
#include "smtk/model/EntityIterator.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Model.h"
#include "smtk/model/Resource.h"

#include "nlohmann/json.hpp"

namespace smtk
{
namespace model
{
using json = nlohmann::json;
// Define how model-entity iterators are serialized.
using UUID = smtk::common::UUID;
using UUIDArray = smtk::common::UUIDArray;
using Entity = smtk::model::Entity;
using EntityRef = smtk::model::EntityRef;
using EntityPtr = smtk::model::EntityPtr;
using KindsToArrangements = smtk::model::KindsToArrangements;
using Models = smtk::model::Models;
using Model = smtk::model::Model;
using ResourcePtr = smtk::model::ResourcePtr;
using EntityIterator = smtk::model::EntityIterator;
using EntityTypeBits = smtk::model::EntityTypeBits;
using IteratorStyle = smtk::model::IteratorStyle;
using Tessellation = smtk::model::Tessellation;

void to_json(json& j, const EntityIterator& iter)
{
  using smtk::model::AbbreviationForArrangementKind;
  EntityIterator mit(iter);
  j = json::object();

  for (mit.begin(); !mit.isAtEnd(); ++mit)
  {
    const EntityRef& ent = mit.current();
    EntityPtr eptr;
    if (ent.isValid(&eptr))
    {
      json jent = { { "d", eptr->dimension() },
        { "e", eptr->entityFlags() }, // Entity::flagToSpecifierString(eptr->entityFlags())
        { "r", eptr->relations() } };
      // Add arrangement information:
      const KindsToArrangements& amap(eptr->arrangementMap());
      if (!amap.empty())
      {
        json jarr = json::object();
        for (auto ktoa : amap)
        {
          jarr[AbbreviationForArrangementKind(ktoa.first)] = ktoa.second;
        }
        jent["a"] = jarr;
      }
      /* Do not include tessellation here
        const Tessellation* tess;
        if ((tess = ent.hasTessellation()))
        {
          jent["t"] = *tess;
        }
         */
      j[ent.entity().toString()] = jent;
    }
  }
}
}
}
