//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/session/polygon/operators/CreateModel.h"

#include "smtk/session/polygon/Resource.h"
#include "smtk/session/polygon/Session.h"
#include "smtk/session/polygon/internal/Model.h"

#include "smtk/io/Logger.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/session/polygon/operators/CreateModel_xml.h"

namespace smtk
{
namespace session
{
namespace polygon
{

CreateModel::Result CreateModel::operateInternal()
{
  // Discover how the user wants to specify scaling.
  smtk::attribute::IntItem::Ptr constructionMethodItem =
    this->parameters()->findInt("construction method");
  int method = constructionMethodItem->discreteIndex(0);

  smtk::attribute::DoubleItem::Ptr originItem = this->parameters()->findDouble("origin");
  smtk::attribute::DoubleItem::Ptr xAxisItem = this->parameters()->findDouble("x axis");
  smtk::attribute::DoubleItem::Ptr yAxisItem = this->parameters()->findDouble("y axis");
  smtk::attribute::DoubleItem::Ptr zAxisItem = this->parameters()->findDouble("z axis");
  smtk::attribute::DoubleItem::Ptr featureSizeItem = this->parameters()->findDouble("feature size");
  smtk::attribute::IntItem::Ptr modelScaleItem = this->parameters()->findInt("model scale");

  internal::pmodel::Ptr storage = internal::pmodel::create();
  bool ok = true;
  // These case values match CreateModel.sbt indices (and enum values):
  switch (method)
  {
    case 0: // origin, 2 axes, and feature size
    {
      std::vector<double> origin(originItem->begin(), originItem->end());
      std::vector<double> x_axis(xAxisItem->begin(), xAxisItem->end());
      std::vector<double> y_axis(yAxisItem->begin(), yAxisItem->end());
      ok = storage->computeModelScaleAndNormal(
        origin, x_axis, y_axis, featureSizeItem->value(0), this->log());
    }
    break;
    case 1: // origin, normal, x axis, and feature size
    {
      std::vector<double> origin(originItem->begin(), originItem->end());
      std::vector<double> x_axis(xAxisItem->begin(), xAxisItem->end());
      std::vector<double> z_axis(zAxisItem->begin(), zAxisItem->end());
      ok = storage->computeModelScaleAndYAxis(
        origin, x_axis, z_axis, featureSizeItem->value(0), this->log());
    }
    break;
    case 2: // origin, 2 axes, and model scale
    {
      std::vector<double> origin(originItem->begin(), originItem->end());
      std::vector<double> x_axis(xAxisItem->begin(), xAxisItem->end());
      std::vector<double> y_axis(yAxisItem->begin(), yAxisItem->end());
      ok = storage->computeFeatureSizeAndNormal(
        origin, x_axis, y_axis, modelScaleItem->value(0), this->log());
    }
    break;
    default:
      ok = false;
      smtkInfoMacro(log(), "Unhandled construction method " << method << ".");
      break;
  }

  Result result;
  if (ok)
  {
    // There are three possible import modes
    //
    // 1. Import a model into an existing resource
    // 2. Import a model as a new model, but using the session of an existing resource
    // 3. Import a model into a new resource

    smtk::session::polygon::Resource::Ptr resource = nullptr;
    smtk::session::polygon::Session::Ptr session = nullptr;

    // Modes 2 and 3 requre an existing resource for input
    smtk::attribute::ReferenceItem::Ptr existingResourceItem = this->parameters()->associations();

    if (existingResourceItem->numberOfValues() > 0)
    {
      smtk::session::polygon::Resource::Ptr existingResource =
        std::static_pointer_cast<smtk::session::polygon::Resource>(existingResourceItem->value());

      session = existingResource->polygonSession();

      smtk::attribute::StringItem::Ptr sessionOnlyItem =
        this->parameters()->findString("session only");
      if (sessionOnlyItem->value() == "this file")
      {
        // If the "session only" value is set to "this file", then we use the
        // existing resource
        resource = existingResource;
      }
      else
      {
        // If the "session only" value is set to "this session", then we create a
        // new resource with the session from the exisiting resource
        resource = smtk::session::polygon::Resource::create();
        resource->setSession(session);
      }
    }
    else
    {
      // If no existing resource is provided, then we create a new session and
      // resource.
      resource = smtk::session::polygon::Resource::create();
      session = smtk::session::polygon::Session::create();

      // Create a new resource for the import
      resource->setSession(session);
    }

    if (session)
    {
      // If a name was specified, use it. Or make one up.
      smtk::attribute::StringItem::Ptr nameItem = this->parameters()->findString("name");
      std::string modelName;
      if (nameItem && nameItem->isEnabled())
      {
        modelName = nameItem->value(0);
      }

      smtk::model::Model model =
        resource->addModel(/* par. dim. */ 2, /* emb. dim. */ 3, modelName);
      storage->setId(model.entity());
      storage->setSession(session);
      resource->addStorage(model.entity(), storage);
      model.setSession(smtk::model::SessionRef(resource, session->sessionId()));
      if (modelName.empty())
      {
        model.assignDefaultName();
      }

      result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

      {
        smtk::attribute::ComponentItem::Ptr resultModels = result->findComponent("model");
        resultModels->setValue(model.component());
      }

      {
        smtk::attribute::ResourceItem::Ptr created = result->findResource("resourcesCreated");
        created->appendValue(resource);
      }

      {
        smtk::attribute::ComponentItem::Ptr created = result->findComponent("created");
        created->setValue(model.component());
      }

      model.setFloatProperty(
        "x axis", smtk::model::FloatList(storage->xAxis(), storage->xAxis() + 3));
      model.setFloatProperty(
        "y axis", smtk::model::FloatList(storage->yAxis(), storage->yAxis() + 3));
      model.setFloatProperty(
        "normal", smtk::model::FloatList(storage->zAxis(), storage->zAxis() + 3));
      model.setFloatProperty(
        "origin", smtk::model::FloatList(storage->origin(), storage->origin() + 3));
      model.setFloatProperty("feature size", storage->featureSize());
      model.setFloatProperty("model scale", storage->modelScale());
      model.setIntegerProperty(SMTK_GEOM_STYLE_PROP, smtk::model::DISCRETE);
    }
  }

  if (!result)
  {
    result = this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  return result;
}

const char* CreateModel::xmlDescription() const
{
  return CreateModel_xml;
}

} // namespace polygon
} //namespace session
} // namespace smtk
