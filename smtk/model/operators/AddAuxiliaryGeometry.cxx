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
#include "smtk/model/Model.h"
#include "smtk/model/Resource.h"
#include "smtk/model/Resource.txx"
#include "smtk/model/Session.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ReferenceItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/operation/MarkGeometry.h"

#include "smtk/model/AddAuxiliaryGeometry_xml.h"

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

AddAuxiliaryGeometry::Result AddAuxiliaryGeometry::operateInternal()
{
  auto associations = this->parameters()->associations();
  auto entities = associations->as<EntityRefArray>(
    [](PersistentObjectPtr obj) { return EntityRef(std::dynamic_pointer_cast<Entity>(obj)); });
  smtk::attribute::FileItemPtr urlItem = this->parameters()->findFile("url");
  if (entities.empty())
  {
    smtkErrorMacro(this->log(), "No " << (urlItem ? "parent" : "children") << " specified.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  EntityRef parent = entities[0];
  smtk::model::Resource::Ptr resource =
    std::static_pointer_cast<smtk::model::Resource>(parent.component()->resource());

  smtk::attribute::StringItemPtr dtypeItem = this->parameters()->findString("type");
  smtk::attribute::IntItemPtr dimItem = this->parameters()->findInt("dimension");
  int dim = dimItem != nullptr ? dimItem->value(0) : 2;

  // Transform
  smtk::attribute::DoubleItemPtr transformItems[3] = { this->parameters()->findDouble("scale"),
                                                       this->parameters()->findDouble("rotate"),
                                                       this->parameters()->findDouble(
                                                         "translate") };
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

  smtk::attribute::VoidItem::Ptr separateRepOption =
    this->parameters()->findVoid("separate representation");
  bool bSeparateRep = separateRepOption->isEnabled();

  AuxiliaryGeometry auxGeom;
  std::vector<EntityRef> reparented;
  if (parent.isModel())
  { // We are in "add auxiliary geometry" and parent is our owning model.
    auxGeom = resource->addAuxiliaryGeometry(parent.as<Model>(), dim);
  }
  else
  { // We are in "composite auxiliary geometry" and should create an aux geom that owns entities.
    parent = parent.owningModel();
    if (parent.isValid())
    {
      auxGeom = resource->addAuxiliaryGeometry(parent.as<AuxiliaryGeometry>(), dim);
      for (const auto& child : entities)
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
  smtk::operation::MarkGeometry(resource).markModified(auxGeom.component());
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
  std::vector<double> bbox;
  if (!urlStr.empty())
  {
    AuxiliaryGeometryExtension::Ptr ext;
    smtk::common::Extension::visit<AuxiliaryGeometryExtension::Ptr>(
      [&ext, &auxGeom, &bbox](const std::string& /*unused*/, AuxiliaryGeometryExtension::Ptr obj) {
        // Don't take the first AuxiliaryGeometryExtenion... look for one
        // that can handle the URL.
        if (obj && obj->canHandleAuxiliaryGeometry(auxGeom, bbox))
        {
          ext = obj;
          return std::make_pair(true, true);
        }
        return std::make_pair(false, false);
      });
    if (ext)
    {
      isURLValid = true;
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
    EntityArray expunged;
    resource->deleteEntities(del, modified, expunged, /*log*/ false);
    smtkErrorMacro(this->log(), "The url \"" << urlStr << "\" is invalid or unhandled.");
    return this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }

  Result result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  smtk::attribute::ComponentItem::Ptr created = result->findComponent("created");
  created->appendValue(auxGeom.component());
  smtk::attribute::ComponentItem::Ptr modified = result->findComponent("modified");
  modified->appendValue(parent.component());
  for (auto& m : reparented)
  {
    modified->appendValue(m.component());
  }

  if (auxGeom.hasURL())
  {
    auto tessItem = result->findComponent("tess_changed");
    tessItem->setNumberOfValues(1);
    tessItem->setValue(auxGeom.component());
  }

  return result;
}

const char* AddAuxiliaryGeometry::xmlDescription() const
{
  return AddAuxiliaryGeometry_xml;
}

} //namespace model
} // namespace smtk
