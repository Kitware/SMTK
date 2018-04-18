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
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/PointLocatorExtension.h"
#include "smtk/model/Session.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include "boost/filesystem.hpp"
SMTK_THIRDPARTY_POST_INCLUDE

using namespace boost::filesystem;
using namespace smtk::model;
using smtk::attribute::FileItem;
using smtk::attribute::StringItem;

namespace smtk
{
namespace model
{

void CreateInstances::addTabularRule(Instance& instance, const EntityRef& prototype)
{
  (void)prototype;
  instance.setRule("tabular");
  smtk::attribute::GroupItem::Ptr placements = this->findGroup("placements");
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
  smtk::attribute::GroupItem::Ptr voi = this->findGroup("volume of interest");
  FloatList vprop;
  vprop.reserve(6);
  for (int axis = 0; axis < 3; ++axis)
  {
    smtk::attribute::DoubleItem::Ptr pt =
      voi->findAs<smtk::attribute::DoubleItem>(axis, "axis range");
    vprop.insert(vprop.end(), pt->begin(), pt->end());
  }
  instance.setFloatProperty("voi", vprop);
  instance.setIntegerProperty("sample size", this->findInt("sample size")->value(0));
  // TODO: Should we accept a seed here as a super-advanced option?
}

void CreateInstances::addSnappingConstraints(Instance& instance, const EntityRef& prototype)
{
  (void)prototype;
  auto snapItem = this->findModelEntity("snap to entity");
  if (snapItem->isEnabled())
  {
    // TODO? Check whether extension is available?
    // auto ext = smtk::common::Extension::findAs<PointLocatorExtension>(
    //   "model_entity_point_locator");
    // Also, if we allow snapping to mesh sets, we should create a different extension.
    // Finally, if we want to allow multiple extensions that perform the same kind of snapping
    // (e.g., when VTK isn't present, some other library might provide point location), we need
    // to handle multiple extensions with either the same name or a way to iterate over extensions
    // by type alone.
    instance.setStringProperty("snap rule", "model_entity_point_locator");
    EntityRefs snapTo(snapItem->begin(), snapItem->end());
    instance.setSnapEntities(snapTo);
  }
}

smtk::model::OperatorResult CreateInstances::operateInternal()
{
  EntityRefArray prototypes = this->associatedEntitiesAs<EntityRefArray>();

  std::string rule = this->findString("placement rule")->value(0);

  smtk::model::EntityRef parent;
  auto parentI = this->findModelEntity("parent");
  if (parentI->isValid() && parentI->isEnabled())
  {
    parent = parentI->value();
  }
  smtk::model::OperatorResult result =
    this->createResult(smtk::operation::Operator::OPERATION_SUCCEEDED);
  for (auto prototype : prototypes)
  {
    ManagerPtr mgr = prototype.manager();
    if (mgr)
    {
      Instance instance = mgr->addInstance(prototype);
      instance.assignDefaultName();
      this->addEntityToResult(result, prototype, MODIFIED);
      this->addEntityToResult(result, instance, CREATED);
      result->findModelEntity("tess_changed")->appendValue(instance);
      if (rule == "tabular")
      {
        this->addTabularRule(instance, prototype);
      }
      else if (rule == "uniform random")
      {
        this->addUniformRandomRule(instance, prototype);
      }
      this->addSnappingConstraints(instance, prototype);
      // Now that the instance is fully specified, generate
      // the placement points.
      instance.generateTessellation();
      // Assign parent relationship
      if (parent.isValid())
      {
        //mgr->arrangeEntity(instance.entity(), )
      }
    }
  }

  return result;
}

} //namespace model
} // namespace smtk

#include "smtk/model/CreateInstances_xml.h"

smtkImplementsModelOperator(SMTKCORE_EXPORT, smtk::model::CreateInstances, create_instances,
  "create instances", CreateInstances_xml, smtk::model::Session);
