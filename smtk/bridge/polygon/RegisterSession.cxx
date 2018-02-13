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
#include "smtk/bridge/polygon/RegisterSession.h"

#include "smtk/bridge/polygon/operators/CleanGeometry.h"
#include "smtk/bridge/polygon/operators/CreateEdge.h"
#include "smtk/bridge/polygon/operators/CreateEdgeFromPoints.h"
#include "smtk/bridge/polygon/operators/CreateEdgeFromVertices.h"
#include "smtk/bridge/polygon/operators/CreateFaces.h"
#include "smtk/bridge/polygon/operators/CreateFacesFromEdges.h"
#include "smtk/bridge/polygon/operators/CreateModel.h"
#include "smtk/bridge/polygon/operators/CreateVertices.h"
#include "smtk/bridge/polygon/operators/Delete.h"
#include "smtk/bridge/polygon/operators/DemoteVertex.h"
#include "smtk/bridge/polygon/operators/ExtractContours.h"
#include "smtk/bridge/polygon/operators/ForceCreateFace.h"
#include "smtk/bridge/polygon/operators/Import.h"
#include "smtk/bridge/polygon/operators/SplitEdge.h"
#include "smtk/bridge/polygon/operators/TweakEdge.h"

#include "smtk/bridge/polygon/Resource.h"
#include "smtk/bridge/polygon/SessionIOJSON.h"

namespace smtk
{
namespace bridge
{
namespace polygon
{

void registerOperations(smtk::operation::Manager::Ptr& operationManager)
{
  operationManager->registerOperation<smtk::bridge::polygon::CleanGeometry>(
    "smtk::bridge::polygon::CleanGeometry");
  operationManager->registerOperation<smtk::bridge::polygon::CreateEdge>(
    "smtk::bridge::polygon::CreateEdge");
  operationManager->registerOperation<smtk::bridge::polygon::CreateEdgeFromPoints>(
    "smtk::bridge::polygon::CreateEdgeFromPoints");
  operationManager->registerOperation<smtk::bridge::polygon::CreateEdgeFromVertices>(
    "smtk::bridge::polygon::CreateEdgeFromVertices");
  operationManager->registerOperation<smtk::bridge::polygon::CreateFaces>(
    "smtk::bridge::polygon::CreateFaces");
  operationManager->registerOperation<smtk::bridge::polygon::CreateFacesFromEdges>(
    "smtk::bridge::polygon::CreateFacesFromEdges");
  operationManager->registerOperation<smtk::bridge::polygon::CreateModel>(
    "smtk::bridge::polygon::CreateModel");
  operationManager->registerOperation<smtk::bridge::polygon::CreateVertices>(
    "smtk::bridge::polygon::CreateVertices");
  operationManager->registerOperation<smtk::bridge::polygon::Delete>(
    "smtk::bridge::polygon::Delete");
  operationManager->registerOperation<smtk::bridge::polygon::DemoteVertex>(
    "smtk::bridge::polygon::DemoteVertex");
  operationManager->registerOperation<smtk::bridge::polygon::ExtractContours>(
    "smtk::bridge::polygon::ExtractContours");
  operationManager->registerOperation<smtk::bridge::polygon::ForceCreateFace>(
    "smtk::bridge::polygon::ForceCreateFace");
  operationManager->registerOperation<smtk::bridge::polygon::Import>(
    "smtk::bridge::polygon::Import");
  operationManager->registerOperation<smtk::bridge::polygon::SplitEdge>(
    "smtk::bridge::polygon::SplitEdge");
  operationManager->registerOperation<smtk::bridge::polygon::TweakEdge>(
    "smtk::bridge::polygon::TweakEdge");
}

void registerResources(smtk::resource::Manager::Ptr& resourceManager)
{
  resourceManager->registerResource<smtk::bridge::polygon::Resource>(
    // ### Load a model file ###
    [](const std::string& filename) -> smtk::resource::ResourcePtr {
      // Load file and parse it:
      smtk::bridge::polygon::SessionIOJSON::json j =
        smtk::bridge::polygon::SessionIOJSON::loadJSON(filename);
      if (j.is_null())
      {
        return smtk::resource::Resource::Ptr();
      }

      // Deserialize parsed JSON into a model resource:
      auto rsrc = smtk::bridge::polygon::Resource::create();
      smtk::bridge::polygon::SessionIOJSON::loadModelRecords(j, rsrc);

      return smtk::static_pointer_cast<smtk::resource::Resource>(rsrc);
    },

    // ### Save a model file ###
    [](const smtk::resource::ResourcePtr& rsrc) -> bool {
      // Serialize rsrc into a set of JSON records:
      smtk::bridge::polygon::SessionIOJSON::json j = smtk::bridge::polygon::SessionIOJSON::saveJSON(
        std::dynamic_pointer_cast<smtk::bridge::polygon::Resource>(rsrc));
      if (j.is_null())
      {
        return false;
      }
      // Write JSON records to the specified URL:
      bool ok = smtk::bridge::polygon::SessionIOJSON::saveModelRecords(j, rsrc->location());
      return ok;
    });
  resourceManager->addLegacyReader(
    "polygon", [](const std::string& filename) -> smtk::resource::ResourcePtr {
      // Load file and parse it:
      smtk::bridge::polygon::SessionIOJSON::json j =
        smtk::bridge::polygon::SessionIOJSON::loadJSON(filename);
      if (j.is_null())
      {
        return smtk::resource::Resource::Ptr();
      }

      // Deserialize parsed JSON into a model resource:
      auto rsrc = smtk::bridge::polygon::Resource::create();
      smtk::bridge::polygon::SessionIOJSON::loadModelRecords(*j.begin(), rsrc);

      return smtk::static_pointer_cast<smtk::resource::Resource>(rsrc);
    });
}
}
}
}
