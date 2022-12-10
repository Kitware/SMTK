//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/operators/CreateInstances.h"

#include "smtk/model/Instance.h"
#include "smtk/model/Model.h"
#include "smtk/model/PointLocatorExtension.h"
#include "smtk/model/Resource.h"
#include "smtk/model/Session.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/operation/MarkGeometry.h"

#include "smtk/model/operators/CreateInstances_xml.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "boost/filesystem.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

using namespace boost::filesystem;
using namespace smtk::model;
using smtk::resource::PersistentObjectPtr;

namespace smtk
{
namespace model
{

void CreateInstances::addTabularRule(Instance& instance, const EntityRef& prototype)
{
  (void)prototype;
  instance.setRule("tabular");
  smtk::attribute::GroupItem::Ptr placements = this->parameters()->findGroup("placements");
  int numPlace = static_cast<int>(placements->numberOfGroups());
  FloatList pprop;
  pprop.reserve(3 * numPlace);
  for (int ii = 0; ii < numPlace; ++ii)
  {
    smtk::attribute::DoubleItem::Ptr pt =
      placements->findAs<smtk::attribute::DoubleItem>(ii, "coordinates");
    pprop.insert(pprop.end(), pt->begin(), pt->end());
  }
  instance.setFloatProperty("placements", pprop);
}

void CreateInstances::addUniformRandomRule(Instance& instance, const EntityRef& prototype)
{
  (void)prototype;
  instance.setRule("uniform random");
  smtk::attribute::GroupItem::Ptr voi = this->parameters()->findGroup("volume of interest");
  FloatList vprop;
  vprop.reserve(6);
  for (int axis = 0; axis < 3; ++axis)
  {
    smtk::attribute::DoubleItem::Ptr pt =
      voi->findAs<smtk::attribute::DoubleItem>(axis, "axis range");
    vprop.insert(vprop.end(), pt->begin(), pt->end());
  }
  instance.setFloatProperty("voi", vprop);
  instance.setIntegerProperty("sample size", this->parameters()->findInt("sample size")->value(0));
  // TODO: Should we accept a seed here as a super-advanced option?
}

void CreateInstances::addUniformRandomOnSurfaceRule(Instance& instance, const EntityRef& prototype)
{
  (void)prototype;
  instance.setRule("uniform random on surface");
  instance.setIntegerProperty("sample size", this->parameters()->findInt("sample size")->value(0));
  auto sampleSurfaceItem = this->parameters()->findComponent("surface");
  instance.setSampleSurface(
    std::dynamic_pointer_cast<smtk::model::Entity>(sampleSurfaceItem->value())
      ->referenceAs<smtk::model::EntityRef>());
  // TODO: Should we accept a seed here as a super-advanced option?
}

void CreateInstances::addSnappingConstraints(Instance& instance, const EntityRef& prototype)
{
  (void)prototype;
  auto snapItem = this->parameters()->findString("snap to entity");
  if (snapItem && snapItem->isEnabled())
  {
    instance.setStringProperty("snap rule", snapItem->value());
    auto snapEntityItem = this->parameters()->findComponent("entity");
    EntityRefs snapTo;
    for (auto it = snapEntityItem->begin(); it != snapEntityItem->end(); ++it)
    {
      snapTo.insert(
        std::static_pointer_cast<smtk::model::Entity>(*it)->referenceAs<smtk::model::EntityRef>());
    }
    instance.setSnapEntities(snapTo);
  }
}

CreateInstances::Result CreateInstances::operateInternal()
{
  auto associations = this->parameters()->associations();
  auto prototypes = associations->as<EntityRefArray>([](PersistentObjectPtr obj) {
    return smtk::model::EntityRef(std::dynamic_pointer_cast<smtk::model::Entity>(obj));
  });

  std::string rule = this->parameters()->findString("placement rule")->value(0);

  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
  smtk::attribute::ComponentItem::Ptr createdItem = result->findComponent("created");
  smtk::attribute::ComponentItem::Ptr modifiedItem = result->findComponent("modified");

  for (const auto& prototype : prototypes)
  {
    smtk::model::Resource::Ptr resource =
      std::static_pointer_cast<smtk::model::Resource>(prototype.component()->resource());

    if (resource)
    {
      Instance instance = resource->addInstance(prototype);
      instance.assignDefaultName();
      createdItem->appendValue(prototype.component());
      modifiedItem->appendValue(instance.component());
      result->findComponent("tess_changed")->appendValue(instance.component());
      smtk::operation::MarkGeometry().markModified(instance.component());
      if (rule == "tabular")
      {
        this->addTabularRule(instance, prototype);
      }
      else if (rule == "uniform random")
      {
        this->addUniformRandomRule(instance, prototype);
      }
      else if (rule == "uniform random on surface")
      {
        this->addUniformRandomOnSurfaceRule(instance, prototype);
      }
      this->addSnappingConstraints(instance, prototype);
      // Now that the instance is fully specified, generate
      // the placement points.
      instance.generateTessellation();
    }
  }

  return result;
}

const char* CreateInstances::xmlDescription() const
{
  return CreateInstances_xml;
}

} //namespace model
} // namespace smtk
