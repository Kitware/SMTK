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
#include "smtk/bridge/mesh/RegisterSession.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"

#include "smtk/bridge/mesh/operators/EulerCharacteristicRatio.h"
#include "smtk/bridge/mesh/operators/ExportOperator.h"
#include "smtk/bridge/mesh/operators/ImportOperator.h"

#include "smtk/bridge/mesh/Resource.h"

#include "smtk/common/Paths.h"

#include "smtk/model/SessionIOJSON.h"

namespace smtk
{
namespace bridge
{
namespace mesh
{

void registerOperations(smtk::operation::Manager::Ptr& operationManager)
{
  operationManager->registerOperator<smtk::bridge::mesh::EulerCharacteristicRatio>(
    "smtk::bridge::mesh::EulerCharacteristicRatio");
  operationManager->registerOperator<smtk::bridge::mesh::ExportOperator>(
    "smtk::bridge::mesh::ExportOperator");
  operationManager->registerOperator<smtk::bridge::mesh::ImportOperator>(
    "smtk::bridge::mesh::ImportOperator");
}

void registerResources(smtk::resource::Manager::Ptr& resourceManager)
{
  resourceManager->registerResource<smtk::bridge::mesh::Resource>(
    [](const std::string& filename) -> smtk::resource::ResourcePtr {
      // Load file and parse it:
      smtk::model::SessionIOJSON::json j = smtk::model::SessionIOJSON::loadJSON(filename);
      if (j.is_null())
      {
        return smtk::resource::Resource::Ptr();
      }

      std::string meshFilename = j.at("Mesh URL");

      // Create an import operator
      smtk::bridge::mesh::ImportOperator::Ptr importOp =
        smtk::bridge::mesh::ImportOperator::create();
      importOp->parameters()->findFile("filename")->setValue(meshFilename);

      // Execute the operation
      smtk::operation::NewOp::Result importOpResult = importOp->operate();

      // Retrieve the resulting resource
      smtk::attribute::ResourceItemPtr resourceItem =
        std::dynamic_pointer_cast<smtk::attribute::ResourceItem>(
          importOpResult->findResource("resource"));

      return resourceItem->value();
    },
    [](const smtk::resource::ResourcePtr& rsrc) -> bool {
      // Downcast the resource into a mesh session resource
      smtk::bridge::mesh::Resource::Ptr resource =
        smtk::dynamic_pointer_cast<smtk::bridge::mesh::Resource>(rsrc);

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

      // Access the model associated with this resource
      smtk::model::Models models =
        resource->entitiesMatchingFlagsAs<smtk::model::Models>(smtk::model::MODEL_ENTITY, false);

      if (models.size() < 1)
      {
        return false;
      }

      smtk::model::Model model = models[0];

      std::string meshFilename = resource->session()->topology(model)->m_collection->location();

      if (meshFilename.empty())
      {
        meshFilename = smtk::common::Paths::stem(rsrc->location()) + ".h5m";
      }

      j["Mesh URL"] = meshFilename;

      smtk::model::SessionIOJSON::saveModelRecords(j, rsrc->location());

      // Create an export operator
      smtk::bridge::mesh::ExportOperator::Ptr exportOp =
        smtk::bridge::mesh::ExportOperator::create();
      exportOp->parameters()->findFile("filename")->setValue(meshFilename);

      // Set the entity association
      exportOp->parameters()->associateEntity(model);

      // Execute the operation
      smtk::operation::NewOp::Result exportOpResult = exportOp->operate();

      // Test for success
      return (exportOpResult->findInt("outcome")->value() ==
        static_cast<int>(smtk::operation::NewOp::Outcome::SUCCEEDED));
    });
}
}
}
}
