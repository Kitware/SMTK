//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/session/rgg/operators/CreateModel.h"

#include "smtk/session/rgg/Resource.h"
#include "smtk/session/rgg/Session.h"

#include "smtk/io/Logger.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/model/Group.h"
#include "smtk/model/Model.h"

#include "smtk/session/rgg/CreateModel_xml.h"
using namespace smtk::model;

namespace smtk
{
namespace session
{
namespace rgg
{
static unsigned int matToColSize = 22;
static struct
{
  std::string mat;
  std::vector<double> color; // rgba
} matToCol[] = { { "No Cell Material", { 1.0, 1.0, 1.0, 1.0 } },
  { "coolant", { 0.3, 0.5, 1.0, .5 } }, { "duct", { 0.3, 0.3, 1.0, .5 } },
  { "follower", { 0.75, 0.2, 0.75, 1.0 } }, { "fuel", { 1.0, 0.1, 0.1, 1.0 } },
  { "gap", { 0.0, 0.0, 0.0, 0.0 } }, { "gasplenum", { 0.3, 1.0, 0.5, 1.0 } },
  { "graphite", { .4, .4, .4, 1.0 } }, { "interassemblygap", { 0.0, 0.0, 0.0, 0.0 } },
  { "metal", { .6, .6, .6, 1.0 } }, { "outerduct", { 0.2, 0.2, 0.2, 1.0 } },
  { "water", { 0.651, 0.741, 0.859, 0.5 } }, { "absorber", { 0.7, 0.2, 0.7, 1.0 } },
  { "activecore", { 1.0, 0.5, 0.3, 1.0 } }, { "cladding", { 0.75, 0.75, 0.75, 1.0 } },
  { "reflector", { 0.5, 0.5, 1.0, 1.0 } }, { "shield", { 0.996, 0.698, 0.298, 1.0 } },
  { "guidetube", { 0.6, 0.6, 0.6, 1.0 } }, { "controlrod", { 0.729, 0.894, 0.702, 1.0 } },
  { "loadpad", { .4, .4, .4, 1.0 } }, { "sodium", { 1.0, 1.0, 0.4, 0.7 } },
  { "restraintring", { .4, .4, .4, 1.0 } } };

CreateModel::Result CreateModel::operateInternal()
{
  Result result;

  // There are two possible create modes
  //
  // 1. Create a model within an existing resource
  // 2. Create a new model, but using the session of an existing resource
  // 3. Import a model into a new resource

  smtk::session::rgg::Resource::Ptr resource = nullptr;
  smtk::session::rgg::Session::Ptr session = nullptr;

  // Modes 2 and 3 requre an existing resource for input
  smtk::attribute::ResourceItem::Ptr existingResourceItem =
    this->parameters()->findResource("resource");

  if (existingResourceItem && existingResourceItem->isEnabled())
  {
    smtk::session::rgg::Resource::Ptr existingResource =
      std::static_pointer_cast<smtk::session::rgg::Resource>(existingResourceItem->value());

    session = existingResource->session();

    smtk::attribute::StringItem::Ptr sessionOnlyItem =
      this->parameters()->findString("session only");
    if (sessionOnlyItem->value() == "import into this file")
    {
      // If the "session only" value is set to "this file", then we use the
      // existing resource
      resource = existingResource;
    }
    else
    {
      // If the "session only" value is set to "this session", then we create a
      // new resource with the session from the exisiting resource
      resource = smtk::session::rgg::Resource::create();
      resource->setSession(session);
    }
  }
  else
  {
    // If no existing resource is provided, then we create a new session and
    // resource.
    resource = smtk::session::rgg::Resource::create();
    session = smtk::session::rgg::Session::create();
    resource->setSession(session);
  }

  smtk::model::Model model = resource->addModel(/* par. dim. */ 3, /* emb. dim. */ 3);
  model.setSession(smtk::model::SessionRef(resource, session->sessionId()));
  model.setIntegerProperty(SMTK_GEOM_STYLE_PROP, smtk::model::DISCRETE);
  if (model.name().empty())
  {
    model.assignDefaultName();
  }

  smtk::model::Group core = resource->addGroup(0, "group"); // Assign the name later
  model.addGroup(core);
  BitFlags mask(0);
  mask |= smtk::model::AUX_GEOM_ENTITY;
  mask |= smtk::model::INSTANCE_ENTITY;
  core.setMembershipMask(mask);

  CreateModel::populateCore(this, core);

  result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);

  {
    smtk::attribute::ResourceItem::Ptr created = result->findResource("resource");
    created->setValue(resource);
  }

  {
    smtk::attribute::ComponentItem::Ptr created = result->findComponent("created");
    created->appendValue(model.component());
    created->appendValue(core.component());
  }

  if (!result)
  {
    result = this->createResult(smtk::operation::Operation::Outcome::FAILED);
  }
  return result;
}

void CreateModel::populateCore(smtk::operation::Operation* op, smtk::model::Group& core)
{
  core.setStringProperty("rggType", SMTK_SESSION_RGG_CORE);
  smtk::attribute::StringItem::Ptr nameItem = op->parameters()->findString("name");
  std::string coreName;
  if (nameItem && nameItem->isEnabled())
  {
    coreName = nameItem->value(0);
  }
  core.setName(coreName);

  smtk::model::Model model = core.owningModel();

  // Hex or rectinlinear
  std::string optype = op->parameters()->findString("geometry type")->value();
  if (optype == "Hex")
  {
    model.setIntegerProperty("hex", 1);
    smtk::attribute::DoubleItemPtr thicknessI = op->parameters()->findDouble("duct thickness");
    smtk::model::FloatList tmp = { thicknessI->value(0), thicknessI->value(0) };
    model.setFloatProperty("duct thickness", tmp);
  }
  else if (optype == "Rect")
  {
    model.setIntegerProperty("hex", 0);
    smtk::attribute::DoubleItemPtr thicknessXI = op->parameters()->findDouble("duct thickness X");
    smtk::attribute::DoubleItemPtr thicknessYI = op->parameters()->findDouble("duct thickness Y");
    smtk::model::FloatList tmp = { thicknessXI->value(0), thicknessYI->value(0) };
    model.setFloatProperty("duct thickness", tmp);
  }
  // Common items for hex and rectilinear core
  smtk::attribute::DoubleItemPtr heightI = op->parameters()->findDouble("height");
  smtk::attribute::DoubleItemPtr zOriginI = op->parameters()->findDouble("z origin");
  if (heightI->numberOfValues() != 1 || zOriginI->numberOfValues() != 1)
  {
    smtkErrorMacro(op->log(), " Fail to set the duct height on the rgg core");
  }
  else
  {
    // SMTK stores the value as zOrigin and (zOrigin + height).
    smtk::model::FloatList tmp = { zOriginI->value(0), zOriginI->value(0) + heightI->value(0) };
    model.setFloatProperty("duct height", tmp);
  }

  smtk::attribute::IntItemPtr latticeSizeItem = op->parameters()->findInt("lattice size");
  if (latticeSizeItem)
  {
    if (latticeSizeItem->numberOfValues() == 1)
    { // Default value condition
      int unitSize = (model.integerProperty("hex")[0]) ? 1 : 4;
      smtk::model::IntegerList size = { unitSize, unitSize };
      model.setIntegerProperty(latticeSizeItem->name(), size);
    }
    else if (latticeSizeItem->numberOfValues() == 2)
    { // Edit core
      smtk::model::IntegerList size(latticeSizeItem->begin(), latticeSizeItem->end());
      model.setIntegerProperty(latticeSizeItem->name(), size);
    }
    else
    {
      smtkErrorMacro(op->log(), "core " << core.name() << " does not have a valid lattice size");
    }
  }
  // Assemblies and their layouts
  // Op would store all assembly uuids as a string property on the core,
  // then for each assembly op would store its layout as an int property.
  smtk::attribute::GroupItemPtr piecesGItem = op->parameters()->findGroup("assemblies and layouts");
  size_t numAssembly;
  if (piecesGItem != nullptr)
  {
    smtk::model::StringList assemblyIds;
    numAssembly = piecesGItem->numberOfGroups();
    for (std::size_t index = 0; index < numAssembly; index++)
    {
      smtk::attribute::StringItemPtr assemblyIdItem =
        piecesGItem->findAs<smtk::attribute::StringItem>(index, "assembly UUID");
      smtk::attribute::IntItemPtr schemaPlanItem =
        piecesGItem->findAs<smtk::attribute::IntItem>(index, "schema plan");
      smtk::attribute::DoubleItemPtr coordsItem =
        piecesGItem->findAs<smtk::attribute::DoubleItem>(index, "coordinates");

      std::string uuid = assemblyIdItem->value();
      assemblyIds.push_back(uuid);
      smtk::model::IntegerList layout(schemaPlanItem->begin(), schemaPlanItem->end());
      model.setIntegerProperty(uuid, layout);
      smtk::model::FloatList coordinates(coordsItem->begin(), coordsItem->end());
      model.setFloatProperty(uuid, coordinates);
    }
    model.setStringProperty("assemblies", assemblyIds);
  }
  else
  {
    smtkErrorMacro(
      op->log(), "core " << coreName << " does not have assemblies and their layouts info");
  }
}

size_t CreateModel::materialNum(smtk::model::Model model)
{
  if (model.isValid() && model.hasStringProperty("materials"))
  { // Check if users have define some materials and they are loaded into SMTK
    smtk::model::StringList materialsList = model.stringProperty("materials");
    return materialsList.size();
  }
  return matToColSize;
}

void CreateModel::getMaterial(const size_t& index, std::string& name, smtk::model::Model model)
{
  if (model.isValid() && model.hasStringProperty("materials"))
  { // Check if users have define some materials and they are loaded into SMTK
    smtk::model::StringList materialsList = model.stringProperty("materials");
    if (index < materialsList.size())
    {
      name = materialsList[index];
      return;
    }
  }
  if (index >= matToColSize)
  {
    return;
  }
  name = matToCol[index].mat;
}

void CreateModel::getMaterialColor(
  const size_t& index, std::vector<double>& rgba, smtk::model::Model model)
{
  if (model.isValid() && model.hasStringProperty("materials"))
  { // Check if users have define some materials and they are loaded into SMTK
    smtk::model::StringList materialsList = model.stringProperty("materials");
    if (index < materialsList.size() && model.hasFloatProperty(materialsList[index]) &&
      (model.floatProperty(materialsList[index]).size() == 4))
    {
      rgba = model.floatProperty(materialsList[index]);
      return;
    }
  }
  if (index >= matToColSize)
  {
    return;
  }
  rgba = matToCol[index].color;
}

const char* CreateModel::xmlDescription() const
{
  return CreateModel_xml;
}
} // namespace rgg
} //namespace session
} // namespace smtk
