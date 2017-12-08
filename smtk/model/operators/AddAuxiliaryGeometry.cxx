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
#include "smtk/model/AuxiliaryGeometryExtension.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Manager.txx"
#include "smtk/model/Model.h"
#include "smtk/model/Session.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
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

smtk::model::OperatorResult AddAuxiliaryGeometry::operateInternal()
{
  EntityRefArray entities = this->associatedEntitiesAs<EntityRefArray>();
  smtk::attribute::FileItemPtr urlItem = this->findFile("url");
  if (entities.empty())
  {
    smtkErrorMacro(this->log(), "No " << (urlItem ? "parent" : "children") << " specified.");
    return this->createResult(smtk::operation::Operator::OPERATION_FAILED);
  }

  EntityRef parent = entities[0];
  smtk::attribute::StringItemPtr dtypeItem = this->findString("type");
  smtk::attribute::IntItemPtr dimItem = this->findInt("dimension");
  int dim = dimItem != nullptr ? dimItem->value(0) : 2;

  // Transform
  smtk::attribute::DoubleItemPtr transformItems[3] = { this->findDouble("scale"),
    this->findDouble("rotate"), this->findDouble("translate") };
  bool transformIsDefault[3] = { true, true, true };
  for (int ii = 0; ii < 3; ++ii)
  {
    if (!transformItems[ii])
    {
      continue;
    }
    for (int jj = 0; jj < 3; ++jj)
    {
      transformIsDefault[ii] &= transformItems[ii]->isUsingDefault(jj);
    }
  }

  smtk::attribute::VoidItem::Ptr separateRepOption = this->findVoid("separate representation");
  bool bSeparateRep = separateRepOption->isEnabled();

  AuxiliaryGeometry auxGeom;
  std::vector<EntityRef> reparented;
  if (parent.isModel())
  { // We are in "add auxiliary geometry" and parent is our owning model.
    auxGeom = parent.manager()->addAuxiliaryGeometry(parent.as<Model>(), dim);
  }
  else
  { // We are in "composite auxiliary geometry" and should create an aux geom that owns entities.
    parent = parent.owningModel();
    if (parent.isValid())
    {
      auxGeom = parent.manager()->addAuxiliaryGeometry(parent.as<AuxiliaryGeometry>(), dim);
      for (auto child : entities)
      {
        if (child.as<smtk::model::AuxiliaryGeometry>().reparent(auxGeom))
        {
          reparented.push_back(child);
        }
      }
    }
  }
  auxGeom.assignDefaultName();
  auxGeom.setIntegerProperty("display as separate representation", bSeparateRep ? 1 : 0);
  // Add transform properties if they are not default values:
  for (int ii = 0; ii < 3; ++ii)
  {
    if (!transformIsDefault[ii])
    {
      auxGeom.setFloatProperty(transformItems[ii]->name(),
        FloatList(transformItems[ii]->begin(), transformItems[ii]->end()));
    }
  }

  if (urlItem && !urlItem->value().empty())
  {
    path filePath(urlItem->value());
    auxGeom.setURL(filePath.string());
    // Grab the file name as the name for the aux geometry
    auxGeom.setName(filePath.filename().string());
  }
  // nullptr check for AddImage operator
  if (dtypeItem != nullptr && !dtypeItem->value(0).empty())
  {
    auxGeom.setStringProperty("type", dtypeItem->value(0));
  }

  std::string urlStr = auxGeom.url();
  bool isURLValid = true; // It can only be invalid if we have a way to test validity.
  bool haveBBox;
  std::vector<double> bbox;
  if (!urlStr.empty())
  {
    AuxiliaryGeometryExtension::Ptr ext;
    smtk::common::Extension::visit<AuxiliaryGeometryExtension::Ptr>(
      [&ext](const std::string&, AuxiliaryGeometryExtension::Ptr obj) {
        if (obj)
        {
          ext = obj;
          return std::make_pair(true, true);
        }
        return std::make_pair(false, false);
      });
    if (ext)
    {
      isURLValid = ext->canHandleAuxiliaryGeometry(auxGeom, bbox);
      haveBBox = true;
    }
    else
    { // We can at least test whether the file exists.
      isURLValid = exists(path(urlStr));
    }
  }

  if (!isURLValid)
  {
    // Delete the auxiliary geometry and fail.
    AuxiliaryGeometries del;
    del.push_back(auxGeom);
    EntityRefArray modified;
    EntityRefArray expunged;
    parent.manager()->deleteEntities(del, modified, expunged, /*log*/ false);
    smtkErrorMacro(this->log(), "The url \"" << urlStr << "\" is invalid or unhandled.");
    return this->createResult(smtk::operation::Operator::OPERATION_FAILED);
  }

  smtk::model::OperatorResult result =
    this->createResult(smtk::operation::Operator::OPERATION_SUCCEEDED);

  this->addEntityToResult(result, parent, MODIFIED);
  this->addEntityToResult(result, auxGeom, CREATED);
  this->addEntitiesToResult(result, reparented, MODIFIED);
  if (auxGeom.hasURL())
  {
    auto tessItem = result->findModelEntity("tess_changed");
    tessItem->setNumberOfValues(1);
    tessItem->setValue(auxGeom);
  }

  return result;
}

} //namespace model
} // namespace smtk

#include "smtk/model/AddAuxiliaryGeometry_xml.h"

smtkImplementsModelOperator(SMTKCORE_EXPORT, smtk::model::AddAuxiliaryGeometry,
  add_auxiliary_geometry, "add auxiliary geometry", AddAuxiliaryGeometry_xml, smtk::model::Session);
