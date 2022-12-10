//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/session/oscillator/operators/EditSource.h"

#include "smtk/session/oscillator/Resource.h"
#include "smtk/session/oscillator/operators/EditSource_xml.h"

#include "smtk/session/mesh/operators/CreateUniformGrid.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/operators/Signal.h"

#include "smtk/operation/Manager.h"

#include "smtk/model/AuxiliaryGeometry.h"
#include "smtk/model/Model.h"
#include "smtk/model/Tessellation.h"

#include "smtk/common/UUID.h"

#include <ctime> // for std::time_t

namespace smtk
{
namespace session
{
namespace oscillator
{

bool EditSource::configure(
  const smtk::attribute::AttributePtr& /*changedAttribute*/,
  const smtk::attribute::ItemPtr& changedItem)
{
  auto params = this->parameters();
  auto assocs = params->associations();
  if (!changedItem || changedItem != assocs || assocs->numberOfValues() != 1)
  {
    return false;
  }
  auto ent = assocs->valueAs<smtk::model::Entity>(0);
  if (!ent)
  {
    return false;
  }

  // Copy their values to local fields
  smtk::model::FloatList center{ 0., 0., 0. };
  smtk::model::FloatList radius{ 0. };

  auto source = ent->referenceAs<smtk::model::AuxiliaryGeometry>();
  if (
    source.isValid() && source.hasStringProperty("oscillator_type") &&
    source.stringProperty("oscillator_type")[0] == "source")
  {
    center = source.floatProperty("center");
    radius = source.floatProperty("radius");
  }
  else
  {
    auto model = ent->referenceAs<smtk::model::Model>();
    if (!model.isValid())
    {
      return false;
    }
    auto bbox = model.boundingBox();
    double minDist = -1.0;
    for (int ii = 0; ii < 3; ++ii)
    {
      center[ii] = 0.5 * (bbox[2 * ii + 0] + bbox[2 * ii + 1]);
      double delta = bbox[2 * ii + 1] - bbox[2 * ii + 0];
      if (delta > 0.0 && (delta > minDist || minDist < 0.0))
      {
        minDist = delta;
      }
    }
    radius[0] = minDist / 4;
  }

  smtk::attribute::DoubleItemPtr centerItem =
    this->parameters()->findGroup("location")->findAs<smtk::attribute::DoubleItem>("center");
  smtk::attribute::DoubleItemPtr radiusItem =
    this->parameters()->findGroup("location")->findAs<smtk::attribute::DoubleItem>("radius");

  for (int ii = 0; ii < 3; ii++)
  {
    centerItem->setValue(ii, center[ii]);
  }
  radiusItem->setValue(0, radius[0]);

  return true;
}

EditSource::Result EditSource::operateInternal()
{
  // Access the origin, size and discretization parameters
  smtk::attribute::DoubleItemPtr centerItem =
    this->parameters()->findGroup("location")->findAs<smtk::attribute::DoubleItem>("center");
  smtk::attribute::DoubleItemPtr radiusItem =
    this->parameters()->findGroup("location")->findAs<smtk::attribute::DoubleItem>("radius");

  // Copy their values to local fields
  smtk::model::FloatList center{ 0., 0., 0. };
  smtk::model::FloatList radius{ 0. };

  for (int i = 0; i < 3; i++)
  {
    center[i] = centerItem->value(i);
  }
  radius[0] = radiusItem->value(0);

  smtk::session::oscillator::Resource::Ptr resource;
  auto inputItem = this->parameters()->associations();
  auto inputValue = inputItem->valueAs<smtk::model::Entity>(0);
  smtk::model::Model model;
  smtk::model::AuxiliaryGeometry source;
  bool modelCreated = false;
  bool sourceCreated = false;
  // We could be provided a model or a model-resource:
  if (inputValue)
  {
    resource =
      std::static_pointer_cast<smtk::session::oscillator::Resource>(inputValue->resource());
    source = inputValue->referenceAs<smtk::model::AuxiliaryGeometry>();
    if (!source.isValid())
    {
      model = inputValue->referenceAs<smtk::model::Model>();
    }
  }
  else
  {
    smtkErrorMacro(this->log(), "Invalid geometry associated as input.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  if (!source.isValid())
  {
    source = resource->addAuxiliaryGeometry(model, /* dimension */ 2);
    source.setStringProperty("oscillator_type", "source");
    EditSource::assignName(model, source);
    sourceCreated = true;
  }

  std::time_t mtime;
  std::time(&mtime);
  source.setFloatProperty("mtime", static_cast<double>(mtime));
  source.setFloatProperty("center", center);
  source.setFloatProperty("radius", radius);

  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  {
    smtk::attribute::ComponentItem::Ptr created = result->findComponent("created");
    smtk::attribute::ComponentItem::Ptr modified = result->findComponent("modified");
    if (modelCreated)
    {
      created->appendValue(model.component());
    }
    else
    {
      modified->appendValue(model.component());
    }
    if (sourceCreated)
    {
      created->appendValue(source.component());
    }
    else
    {
      modified->appendValue(source.component());
    }
  }

  return result;
}

const char* EditSource::xmlDescription() const
{
  return EditSource_xml;
}

void EditSource::assignName(smtk::model::Model& model, smtk::model::AuxiliaryGeometry& source)
{
  int number = 1;
  if (model.hasIntegerProperty("source counter"))
  {
    auto& sc = model.integerProperty("source counter");
    number = sc[0]++;
  }
  else
  {
    model.setIntegerProperty("source counter", number + 1);
  }

  std::ostringstream name;
  name << "source " << number;
  source.setName(name.str());
}
} // namespace oscillator
} // namespace session
} // namespace smtk
