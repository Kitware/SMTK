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
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Session.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

using namespace smtk::model;
using smtk::attribute::FileItem;
using smtk::attribute::StringItem;

namespace smtk
{
namespace model
{

smtk::model::OperatorResult GroupAuxiliaryGeometry::operateInternal()
{
  auto entities = this->associatedEntitiesAs<AuxiliaryGeometries>();
  if (entities.empty())
  {
    smtkErrorMacro(this->log(), "No children specified.");
    return this->createResult(smtk::model::OPERATION_FAILED);
  }

  Model parent = entities[0].owningModel();
  smtk::attribute::StringItemPtr nameItem = this->findString("name");
  smtk::attribute::IntItemPtr dimItem = this->findInt("dimension");
  int dim = dimItem != nullptr ? dimItem->value(0) : -1;
  if (dim < 0)
  { // Find dimension from children
    for (auto ent : entities)
    {
      int edim = ent.dimension();
      if (edim > dim)
      {
        dim = edim;
      }
    }
  }

  // Transform
  smtk::attribute::DoubleItemPtr transformItems[3] = { this->findDouble("scale"),
    this->findDouble("rotate"), this->findDouble("translate") };
  bool transformIsDefault[3] = { true, true, true };
  for (int ii = 0; ii < 3; ++ii)
  {
    for (int jj = 0; jj < 3; ++jj)
    {
      transformIsDefault[ii] &= transformItems[ii]->isUsingDefault(jj);
    }
  }

  AuxiliaryGeometry auxGeom;
  auxGeom = parent.manager()->addAuxiliaryGeometry(parent, dim);
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
      auxGeom.setFloatProperty(transformItems[ii]->name(),
        FloatList(transformItems[ii]->begin(), transformItems[ii]->end()));
    }
  }

  auxGeom.setStringProperty("type", "group");

  smtk::model::OperatorResult result = this->createResult(smtk::model::OPERATION_SUCCEEDED);

  this->addEntityToResult(result, parent, MODIFIED);
  this->addEntitiesToResult(result, entities, MODIFIED);
  this->addEntityToResult(result, auxGeom, CREATED);
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

} //namespace model
} // namespace smtk

#include "smtk/model/GroupAuxiliaryGeometry_xml.h"

smtkImplementsModelOperator(SMTKCORE_EXPORT, smtk::model::GroupAuxiliaryGeometry,
  group_auxiliary_geometry, "group auxiliary geometry", GroupAuxiliaryGeometry_xml,
  smtk::model::Session);
