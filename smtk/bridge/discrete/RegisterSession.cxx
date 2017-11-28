//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================
#include "smtk/bridge/discrete/RegisterSession.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"

#include "smtk/bridge/discrete/operators/CreateEdgesOperator.h"
#include "smtk/bridge/discrete/operators/EdgeOperator.h"
#include "smtk/bridge/discrete/operators/EntityGroupOperator.h"
#include "smtk/bridge/discrete/operators/GrowOperator.h"
#include "smtk/bridge/discrete/operators/ImportOperator.h"
#include "smtk/bridge/discrete/operators/MergeOperator.h"
#include "smtk/bridge/discrete/operators/ReadOperator.h"
#include "smtk/bridge/discrete/operators/RemoveModel.h"
#include "smtk/bridge/discrete/operators/SetProperty.h"
#include "smtk/bridge/discrete/operators/SplitFaceOperator.h"
#include "smtk/bridge/discrete/operators/WriteOperator.h"

#include "smtk/bridge/discrete/Resource.h"

#include "smtk/model/SessionIOJSON.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "boost/filesystem.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

using namespace boost::filesystem;

namespace smtk
{
namespace bridge
{
namespace discrete
{

void registerOperations(smtk::operation::Manager::Ptr& operationManager)
{
  operationManager->registerOperator<smtk::bridge::discrete::CreateEdgesOperator>(
    "smtk::bridge::discrete::CreateEdgesOperator");
  operationManager->registerOperator<smtk::bridge::discrete::EdgeOperator>(
    "smtk::bridge::discrete::EdgeOperator");
  operationManager->registerOperator<smtk::bridge::discrete::EntityGroupOperator>(
    "smtk::bridge::discrete::EntityGroupOperator");
  operationManager->registerOperator<smtk::bridge::discrete::GrowOperator>(
    "smtk::bridge::discrete::GrowOperator");
  operationManager->registerOperator<smtk::bridge::discrete::ImportOperator>(
    "smtk::bridge::discrete::ImportOperator");
  operationManager->registerOperator<smtk::bridge::discrete::MergeOperator>(
    "smtk::bridge::discrete::MergeOperator");
  operationManager->registerOperator<smtk::bridge::discrete::ReadOperator>(
    "smtk::bridge::discrete::ReadOperator");
  operationManager->registerOperator<smtk::bridge::discrete::RemoveModel>(
    "smtk::bridge::discrete::RemoveModel");
  operationManager->registerOperator<smtk::bridge::discrete::SetProperty>(
    "smtk::bridge::discrete::SetProperty");
  operationManager->registerOperator<smtk::bridge::discrete::SplitFaceOperator>(
    "smtk::bridge::discrete::SplitFaceOperator");
  operationManager->registerOperator<smtk::bridge::discrete::WriteOperator>(
    "smtk::bridge::discrete::WriteOperator");
}

void registerResources(smtk::resource::Manager::Ptr& resourceManager)
{
  resourceManager->registerResource<smtk::bridge::discrete::Resource>(
    // ### Load a model file ###
    [](const std::string& filename) -> smtk::resource::ResourcePtr {
      // Load file and parse it:
      smtk::model::SessionIOJSON::json j = smtk::model::SessionIOJSON::loadJSON(filename);
      if (j.is_null())
      {
        return smtk::resource::Resource::Ptr();
      }

      // Deserialize parsed JSON into a model resource:
      auto resource = smtk::bridge::discrete::Resource::create();
      smtk::model::SessionIOJSON::loadModelRecords(j, resource);

      // Access the model associated with this resource
      smtk::model::Models models =
        resource->entitiesMatchingFlagsAs<smtk::model::Models>(smtk::model::MODEL_ENTITY, false);

      for (auto& model : models)
      {
        smtk::common::UUID modelid = model.entity();

        std::string nativemodelfile;
        std::string nativefilekey = resource->hasStringProperty(modelid, "url") ? "url" : "";
        if (!nativefilekey.empty())
        {
          smtk::model::StringList const& nprop(resource->stringProperty(modelid, nativefilekey));
          if (!nprop.empty())
          {
            nativemodelfile = nprop[0];
          }

          if (!nativemodelfile.empty())
          {
            smtk::bridge::discrete::ReadOperator::Ptr readOp =
              smtk::bridge::discrete::ReadOperator::create();

            readOp->parameters()->findFile("filename")->setValue(nativemodelfile);

            // Set the entity association
            readOp->parameters()->associateEntity(model);

            // Execute the operation
            smtk::operation::NewOp::Result readOpResult = readOp->operate();
          }
        }
      }
      return smtk::static_pointer_cast<smtk::resource::Resource>(resource);
    },

    // ### Save a model file ###
    [](const smtk::resource::ResourcePtr& rsrc) -> bool {
      smtk::model::ManagerPtr resource = std::dynamic_pointer_cast<smtk::model::Manager>(rsrc);
      if (resource == nullptr)
      {
        return false;
      }

      // Serialize rsrc into a set of JSON records:
      smtk::model::SessionIOJSON::json j = smtk::model::SessionIOJSON::saveJSON(resource);
      if (j.is_null())
      {
        return false;
      }
      // Write JSON records to the specified URL:
      bool ok = smtk::model::SessionIOJSON::saveModelRecords(j, rsrc->location());

      // Access the model associated with this resource
      smtk::model::Models models =
        resource->entitiesMatchingFlagsAs<smtk::model::Models>(smtk::model::MODEL_ENTITY, false);

      for (auto& model : models)
      {
        smtk::common::UUID modelid = model.entity();

        std::string nativemodelfile;
        std::string nativefilekey = resource->hasStringProperty(modelid, "url") ? "url" : "";
        if (!nativefilekey.empty())
        {
          smtk::model::StringList const& nprop(resource->stringProperty(modelid, nativefilekey));
          if (!nprop.empty())
          {
            nativemodelfile = nprop[0];
          }

          if (!nativemodelfile.empty())
          {
            smtk::bridge::discrete::WriteOperator::Ptr writeOp =
              smtk::bridge::discrete::WriteOperator::create();

            writeOp->parameters()->findFile("filename")->setValue(nativemodelfile);

            // Set the entity association
            writeOp->parameters()->associateEntity(model);

            // Execute the operation
            smtk::operation::NewOp::Result writeOpResult = writeOp->operate();

            // Test for success
            ok &= (writeOpResult->findInt("outcome")->value() ==
              static_cast<int>(smtk::operation::NewOp::Outcome::SUCCEEDED));
          }
        }
      }

      return ok;
    });
  resourceManager->addLegacyReader(
    "discrete", [](const std::string& filename) -> smtk::resource::ResourcePtr {
      // Load file and parse it:
      smtk::model::SessionIOJSON::json j = smtk::model::SessionIOJSON::loadJSON(filename);
      if (j.is_null())
      {
        return smtk::resource::Resource::Ptr();
      }

      // Deserialize parsed JSON into a model resource:
      auto resource = smtk::bridge::discrete::Resource::create();
      smtk::model::SessionIOJSON::loadModelRecords(*j.begin(), resource);

      // Access the model associated with this resource
      smtk::model::Models models =
        resource->entitiesMatchingFlagsAs<smtk::model::Models>(smtk::model::MODEL_ENTITY, false);

      for (auto& model : models)
      {
        smtk::common::UUID modelid = model.entity();

        std::string nativemodelfile;
        std::string nativefilekey = resource->hasStringProperty(modelid, "url") ? "url" : "";
        if (!nativefilekey.empty())
        {
          smtk::model::StringList const& nprop(resource->stringProperty(modelid, nativefilekey));
          if (!nprop.empty())
          {
            nativemodelfile = nprop[0];
          }

          if (!nativemodelfile.empty())
          {
            smtk::bridge::discrete::ReadOperator::Ptr readOp =
              smtk::bridge::discrete::ReadOperator::create();

            readOp->parameters()->findFile("filename")->setValue(nativemodelfile);

            // Set the entity association
            readOp->parameters()->associateEntity(model);

            // Execute the operation
            smtk::operation::NewOp::Result readOpResult = readOp->operate();
          }
        }
      }
      return smtk::static_pointer_cast<smtk::resource::Resource>(resource);
    });
}
}
}
}
