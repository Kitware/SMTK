//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/operators/GroupAuxiliaryGeometry.h"

#include "smtk/model/AuxiliaryGeometry.h"
#include "smtk/model/Model.h"
#include "smtk/model/Resource.h"
#include "smtk/model/Session.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/model/operators/GroupAuxiliaryGeometry_xml.h"

using namespace smtk::model;

namespace smtk
{
namespace model
{

GroupAuxiliaryGeometry::Result GroupAuxiliaryGeometry::operateInternal()
{
  auto associations = this->parameters()->associations();
  auto entities =
    associations->as<AuxiliaryGeometries>([](smtk::resource::PersistentObjectPtr obj) {
      return smtk::model::AuxiliaryGeometry(std::dynamic_pointer_cast<smtk::model::Entity>(obj));
    });
  if (entities.empty())
  {
    smtkErrorMacro(this->log(), "No children specified.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  Model parent = entities[0].owningModel();
  smtk::attribute::StringItemPtr nameItem = this->parameters()->findString("name");
  smtk::attribute::IntItemPtr dimItem = this->parameters()->findInt("dimension");
  int dim = dimItem != nullptr ? dimItem->value(0) : -1;
  if (dim < 0)
  { // Find dimension from children
    for (const auto& ent : entities)
    {
      int edim = ent.dimension();
      if (edim > dim)
      {
        dim = edim;
      }
    }
  }

  // Transform
  smtk::attribute::DoubleItemPtr transformItems[3] = { this->parameters()->findDouble("scale"),
                                                       this->parameters()->findDouble("rotate"),
                                                       this->parameters()->findDouble(
                                                         "translate") };
  bool transformIsDefault[3] = { true, true, true };
  for (int ii = 0; ii < 3; ++ii)
  {
    for (int jj = 0; jj < 3; ++jj)
    {
      transformIsDefault[ii] &= transformItems[ii]->isUsingDefault(jj);
    }
  }

  AuxiliaryGeometry auxGeom;
  auxGeom = parent.resource()->addAuxiliaryGeometry(parent, dim);
  for (auto child : entities)
  {
    child.reparent(auxGeom);
  }
  if (nameItem->isEnabled())
  {
    auxGeom.setName(nameItem->value(0));
  }
  else
  {
    auxGeom.assignDefaultName();
  }
  auxGeom.setIntegerProperty("display as separate representation", 0);
  // Add transform properties if they are not default values:
  for (int ii = 0; ii < 3; ++ii)
  {
    if (!transformIsDefault[ii])
    {
      auxGeom.setFloatProperty(
        transformItems[ii]->name(),
        FloatList(transformItems[ii]->begin(), transformItems[ii]->end()));
    }
  }

  auxGeom.setStringProperty("type", "group");

  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  smtk::attribute::ComponentItem::Ptr created = result->findComponent("created");
  created->setValue(auxGeom.component());
  smtk::attribute::ComponentItem::Ptr modified = result->findComponent("modified");
  modified->appendValue(parent.component());
  for (const auto& m : entities)
  {
    modified->appendValue(m.component());
  }
  /*
   * In theory, we might need to set tess_changed on children
   * if their new parent has a non-default transform applied.
  if (transformIsDefault[0 or 1 or 2])
  {
    for (child : entities) { result->findModelEntity("tess_changed")->setValue(child); }
  }
  */

  return result;
}

const char* GroupAuxiliaryGeometry::xmlDescription() const
{
  return GroupAuxiliaryGeometry_xml;
}

} //namespace model
} // namespace smtk
