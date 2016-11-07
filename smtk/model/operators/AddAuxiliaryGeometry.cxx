//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/model/operators/AddAuxiliaryGeometry.h"

#include "smtk/model/AuxiliaryGeometry.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Session.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/VoidItem.h"

using namespace smtk::model;
using smtk::attribute::FileItem;
using smtk::attribute::StringItem;

namespace smtk {
  namespace model {

smtk::model::OperatorResult AddAuxiliaryGeometry::operateInternal()
{
  EntityRefArray entities = this->associatedEntitiesAs<EntityRefArray>();
  if (entities.empty())
    {
    smtkErrorMacro(this->log(), "No parent specified.");
    return this->createResult(smtk::model::OPERATION_FAILED);
    }

  EntityRef parent = entities[0];
  smtk::attribute::FileItemPtr urlItem = this->findFile("url");
  smtk::attribute::StringItemPtr dtypeItem = this->findString("type");
  smtk::attribute::IntItemPtr dimItem = this->findInt("dimension");
  int dim = dimItem->value(0);

  smtk::attribute::VoidItem::Ptr separateRepOption =
    this->findVoid("DisplayAsSeparateRepresentation");
  bool bSeparateRep = separateRepOption->isEnabled();

  AuxiliaryGeometry auxGeom;
  if (parent.isModel())
    {
    auxGeom = parent.manager()->addAuxiliaryGeometry(parent.as<Model>(), dim);
    }
  else
    {
    auxGeom = parent.manager()->addAuxiliaryGeometry(parent.as<AuxiliaryGeometry>(), dim);
    }
  auxGeom.assignDefaultName();
  auxGeom.setIntegerProperty("display as separate representation", bSeparateRep ? 1 : 0);

  if (urlItem->numberOfValues() > 0 && !urlItem->value(0).empty())
    {
    auxGeom.setUrl(urlItem->value(0));
    }
  if (!dtypeItem->value(0).empty())
    {
    auxGeom.setStringProperty("type", dtypeItem->value(0));
    }

  smtk::model::OperatorResult result = this->createResult(
    smtk::model::OPERATION_SUCCEEDED);

  this->addEntityToResult(result, parent, MODIFIED);
  this->addEntityToResult(result, auxGeom, CREATED);
  if (auxGeom.hasUrl())
    {
    result->findModelEntity("tess_changed")->setValue(auxGeom);
    }

  return result;
}

  } //namespace model
} // namespace smtk

#include "smtk/model/AddAuxiliaryGeometry_xml.h"

smtkImplementsModelOperator(
  SMTKCORE_EXPORT,
  smtk::model::AddAuxiliaryGeometry,
  add_auxiliary_geometry,
  "add auxiliary geometry",
  AddAuxiliaryGeometry_xml,
  smtk::model::Session);
