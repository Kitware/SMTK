//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/json/jsonResource.h"

#include "smtk/common/json/jsonUUID.h"
#include "smtk/model/json/jsonArrangement.h"
#include "smtk/model/json/jsonTessellation.h"

#include "smtk/model/Arrangement.h"
#include "smtk/model/CellEntity.h"
#include "smtk/model/EntityIterator.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/Group.h"
#include "smtk/model/Model.h"
#include "smtk/model/Resource.h"

#include "smtk/resource/json/jsonResource.h"

#include "nlohmann/json.hpp"

// Define how model collections are serialized.
namespace smtk
{
namespace model
{
using UUID = smtk::common::UUID;
using UUIDArray = smtk::common::UUIDArray;
using CellEntity = smtk::model::CellEntity;
using Entity = smtk::model::Entity;
using EntityRef = smtk::model::EntityRef;
using EntityPtr = smtk::model::EntityPtr;
using Group = smtk::model::Group;
using KindsToArrangements = smtk::model::KindsToArrangements;
using Models = smtk::model::Models;
using Model = smtk::model::Model;
using ResourcePtr = smtk::model::ResourcePtr;
using EntityIterator = smtk::model::EntityIterator;
using EntityTypeBits = smtk::model::EntityTypeBits;
using IteratorStyle = smtk::model::IteratorStyle;
using Tessellation = smtk::model::Tessellation;

void to_json(json& j, const ResourcePtr& mresource)
{
  smtk::resource::to_json(j, smtk::static_pointer_cast<smtk::resource::Resource>(mresource));

  j["version"] = "3.1";

  using smtk::model::AbbreviationForArrangementKind;
  json jmodels = json::object();

  Models models = mresource->entitiesMatchingFlagsAs<Models>(EntityTypeBits::MODEL_ENTITY, false);
  for (const auto& model : models)
  {
    json jmodel = json::object();
    EntityIterator eit;
    eit.traverse(model, IteratorStyle::ITERATE_CHILDREN);
    for (eit.begin(); !eit.isAtEnd(); ++eit)
    {
      const EntityRef& ent = eit.current();
      EntityPtr eptr;
      if (ent.isValid(&eptr))
      {
        json jent = { { "d", eptr->dimension() },
                      { "e",
                        eptr->entityFlags() }, // Entity::flagToSpecifierString(eptr->entityFlags())
                      { "r", eptr->relations() } };
        // Add arrangement information:
        const KindsToArrangements& amap(eptr->arrangementMap());
        if (!amap.empty())
        {
          json jarr = json::object();
          for (const auto& ktoa : amap)
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
        jmodel[ent.entity().toString()] = jent;
      }
    }
    jmodels[model.entity().toString()] = jmodel;
  }
  j["models"] = jmodels;
}

void from_json(const json& j, ResourcePtr& mresource)
{
  if (!mresource)
  {
    // It's derived class's responsibility to create a valid mresource
    return;
  }

  auto temp = std::static_pointer_cast<smtk::resource::Resource>(mresource);
  smtk::resource::from_json(j, temp);

  json jmodels;
  try
  {
    jmodels = j.at("models");
  }
  catch (std::exception& /*e*/)
  {
    std::cerr << "Models does not exist in resource json object" << std::endl;
    return;
  }
  for (json::const_iterator jmodelIt = jmodels.begin(); jmodelIt != jmodels.end(); ++jmodelIt)
  { // Models
    UUID mid = UUID(jmodelIt.key());
    json jModel = jmodelIt.value();
    auto jModelInfo = jModel.find(jmodelIt.key());
    BitFlags modelBitFlags = jModelInfo->at("e");
    EntityPtr currentModelPtr = Entity::create(mid, modelBitFlags, mresource);
    mresource->addEntity(currentModelPtr);
    Model currentModel = currentModelPtr->referenceAs<Model>();
    for (auto jentIt = jModel.begin(); jentIt != jModel.end(); jentIt++)
    { // Entities in the currentModel
      // Create and fill in the entity first
      UUID eid = UUID(jentIt.key());
      json jEntity = jentIt.value();
      BitFlags bitflags;
      try
      {
        bitflags = jEntity.at("e");
      }
      catch (std::exception&)
      {
        std::cerr << "Failed to add entityFlags to entity " << eid.toString() << std::endl;
        continue;
      }
      EntityPtr entity = Entity::create(eid, bitflags, mresource);
      mresource->addEntity(entity);
      EntityRef entRef = EntityRef(mresource, eid);

      try
      {
        UUIDArray uuidArray = jEntity.at("r");
        entity->relations() = uuidArray;
      }
      catch (std::exception&)
      {
      }

      // Add arrangementInfo
      try
      {
        json arrangementMap = jEntity.at("a");
        entity->clearArrangements();
        for (auto arrIter = arrangementMap.begin(); arrIter != arrangementMap.end(); arrIter++)
        {
          ArrangementKind kind = ArrangementKindFromAbbreviation(std::string(arrIter.key()));
          Arrangements arrangements = arrIter.value();
          for (const auto& arrangement : arrangements)
          {
            entity->arrange(kind, arrangement);
          }
        }
      }
      catch (std::exception&)
      {
      }
      // For now from_json would just replace the ${Type}Properties for current entity
      try
      {
        json stringDataJ = jEntity.at("s");
        // Nlohmann json does not support complicated map conversion, we need
        // to do it manually
        for (auto sdIt = stringDataJ.begin(); sdIt != stringDataJ.end(); sdIt++)
        {
          auto& stringProperties =
            mresource->properties()
              .data()
              .get<std::unordered_map<smtk::common::UUID, std::vector<std::string>>>();
          stringProperties[sdIt.key()][eid] = sdIt.value().get<StringList>();
        }
      }
      catch (std::exception&)
      {
      }
      try
      {
        json floatDataJ = jEntity.at("f");
        // Nlohmann json does not support complicated map conversion, we need
        // to do it manually
        for (auto sdIt = floatDataJ.begin(); sdIt != floatDataJ.end(); sdIt++)
        {
          auto& floatProperties =
            mresource->properties()
              .data()
              .get<std::unordered_map<smtk::common::UUID, std::vector<double>>>();
          floatProperties[sdIt.key()][eid] = sdIt.value().get<FloatList>();
        }
      }
      catch (std::exception&)
      {
      }
      try
      {
        json intDataJ = jEntity.at("i");
        // Nlohmann json does not support complicated map conversion, we need
        // to do it manually
        for (auto sdIt = intDataJ.begin(); sdIt != intDataJ.end(); sdIt++)
        {
          auto& intProperties = mresource->properties()
                                  .data()
                                  .get<std::unordered_map<smtk::common::UUID, std::vector<long>>>();
          intProperties[sdIt.key()][eid] = sdIt.value().get<IntegerList>();
        }
      }
      catch (std::exception&)
      {
      }
    }
  }
}
} // namespace model
} // namespace smtk
