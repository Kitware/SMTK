//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/bridge/rgg/operators/CreateModel.h"

#include "smtk/bridge/rgg/Session.h"

#include "smtk/io/Logger.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"

#include "smtk/model/Group.h"
#include "smtk/model/Model.h"

#include "smtk/bridge/rgg/CreateModel_xml.h"
using namespace smtk::model;

namespace smtk
{
namespace bridge
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

smtk::model::OperatorResult CreateModel::operateInternal()
{
  smtk::model::OperatorResult result;
  smtk::bridge::rgg::SessionPtr sess = this->activeSession();
  smtk::model::Manager::Ptr mgr;
  if (sess)
  {
    mgr = sess->manager();
    smtk::model::Model model = mgr->addModel(/* par. dim. */ 3, /* emb. dim. */ 3);
    model.setSession(smtk::model::SessionRef(mgr, sess->sessionId()));
    if (model.name().empty())
    {
      model.assignDefaultName();
    }

    smtk::model::Group core = mgr->addGroup(0, "group"); // Assign the name later
    model.addGroup(core);
    BitFlags mask(0);
    mask |= smtk::model::AUX_GEOM_ENTITY;
    mask |= smtk::model::INSTANCE_ENTITY;
    core.setMembershipMask(mask);

    CreateModel::populateCore(this, core);

    result = this->createResult(smtk::operation::Operator::OPERATION_SUCCEEDED);
    this->addEntityToResult(result, model, CREATED);
    this->addEntityToResult(result, core, CREATED);
    model.setIntegerProperty(SMTK_GEOM_STYLE_PROP, smtk::model::DISCRETE);
  }

  if (!result)
  {
    result = this->createResult(smtk::operation::Operator::OPERATION_FAILED);
  }
  return result;
}

void CreateModel::populateCore(smtk::model::Operator* op, smtk::model::Group& core)
{
  core.setStringProperty("rggType", SMTK_BRIDGE_RGG_CORE);
  smtk::attribute::StringItem::Ptr nameItem = op->findString("name");
  std::string coreName;
  if (nameItem && nameItem->isEnabled())
  {
    coreName = nameItem->value(0);
  }
  core.setName(coreName);

  smtk::model::Model model = core.owningModel();

  // Hex or rectinlinear
  std::string optype = op->findString("geometry type")->value();
  if (optype == "Hex")
  {
    model.setIntegerProperty("hex", 1);
    smtk::attribute::DoubleItemPtr thicknessI = op->findDouble("duct thickness");
    smtk::model::FloatList tmp = { thicknessI->value(0), thicknessI->value(0) };
    model.setFloatProperty("duct thickness", tmp);
  }
  else if (optype == "Rect")
  {
    model.setIntegerProperty("hex", 0);
    smtk::attribute::DoubleItemPtr thicknessXI = op->findDouble("duct thickness X");
    smtk::attribute::DoubleItemPtr thicknessYI = op->findDouble("duct thickness Y");
    smtk::model::FloatList tmp = { thicknessXI->value(0), thicknessYI->value(0) };
    model.setFloatProperty("duct thickness", tmp);
  }
  // Common items for hex and rectilinear core
  smtk::attribute::DoubleItemPtr heightI = op->findDouble("height");
  smtk::attribute::DoubleItemPtr zOriginI = op->findDouble("z origin");
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

  smtk::attribute::IntItemPtr latticeSizeItem = op->findInt("lattice size");
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
  smtk::attribute::GroupItemPtr piecesGItem = op->findGroup("assemblies and layouts");
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

} // namespace rgg
} //namespace bridge
} // namespace smtk

smtkImplementsModelOperator(SMTKRGGSESSION_EXPORT, smtk::bridge::rgg::CreateModel, rgg_create_model,
  "create model", CreateModel_xml, smtk::bridge::rgg::Session);
