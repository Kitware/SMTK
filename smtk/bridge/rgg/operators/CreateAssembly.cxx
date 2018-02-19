//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/bridge/rgg/operators/CreateAssembly.h"

#include "smtk/bridge/rgg/Session.h"

#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"
#include "smtk/io/Logger.h"

#include "smtk/model/Group.h"

#include "smtk/bridge/rgg/CreateAssembly_xml.h"

using namespace smtk::model;

namespace smtk
{
namespace bridge
{
namespace rgg
{

smtk::model::OperatorResult CreateAssembly::operateInternal()
{
  smtk::model::OperatorResult result =
    this->createResult(smtk::operation::Operator::OPERATION_FAILED);
  smtk::bridge::rgg::SessionPtr sess = this->activeSession();
  if (!sess)
  {
    return result;
  }
  EntityRefArray entities = this->associatedEntitiesAs<EntityRefArray>();
  // Since we are in creating mode, it must be a model
  if (entities.empty() || !entities[0].isModel())
  {
    smtkErrorMacro(this->log(), "An invalid model is provided for create assembly op");
    return this->createResult(smtk::operation::Operator::OPERATION_FAILED);
  }

  EntityRef parent = entities[0];
  smtk::model::Group assembly = parent.manager()->addGroup(0, "group"); // Assign the name later
  CreateAssembly::populateAssembly(this, assembly, true);
  smtk::model::Model model = entities[0].as<smtk::model::Model>();
  model.addGroup(assembly);

  result = this->createResult(smtk::operation::Operator::OPERATION_SUCCEEDED);

  this->addEntityToResult(result, assembly, CREATED);
  return result;
}

void CreateAssembly::populateAssembly(
  smtk::model::Operator* op, smtk::model::Group& assembly, bool createMode)
{
  /******************************************************/
  std::cout << "CreateAssembly::populateAssembly" << std::endl;
  /******************************************************/
  assembly.setStringProperty("rggType", SMTK_BRIDGE_RGG_ASSEMBLY);
  smtk::attribute::StringItemPtr nameItem = op->findString("name");
  std::string assName;
  if (nameItem != nullptr && !nameItem->value(0).empty())
  {
    assName = nameItem->value(0);
    assembly.setName(nameItem->value(0));
  }
  /******************************************************/
  std::cout << "  name=" << assName << std::endl;
  /******************************************************/

  smtk::attribute::StringItemPtr labelItem = op->findString("label");
  if (labelItem != nullptr && !labelItem->value(0).empty())
  {
    smtk::model::Model model = assembly.owningModel();
    std::string labelValue = labelItem->value(0);
    if (model.hasStringProperty("assembly labels list"))
    {
      smtk::model::StringList& labels = model.stringProperty("assembly labels list");
      int count = 0;
      while (std::find(std::begin(labels), std::end(labels), labelValue) != std::end(labels))
      { // need to generate a new label
        labelValue = "assembly" + std::to_string(count);
        count++;
      }
      // Update the label list
      labels.push_back(labelValue);
    }
    else
    {
      model.setStringProperty("assembly labels list", labelValue);
    }
    assembly.setStringProperty(labelItem->name(), labelValue);
    /******************************************************/
    std::cout << "  label=" << labelValue << std::endl;
    /******************************************************/
  }

  // Hex or rectinlinear should and could only be decided at creation time
  smtk::attribute::VoidItemPtr isHex = op->findVoid("hex");
  if (isHex->isEnabled() && createMode)
  {
    assembly.setIntegerProperty(isHex->name(), 1);
  }
  else if (!isHex->isEnabled() && createMode)
  {
    assembly.setIntegerProperty(isHex->name(), 0);
  }
  /******************************************************/
  std::cout << "  hex=" << isHex->isEnabled() << std::endl;
  /******************************************************/
  // Pins and their layouts
  // Op would store all pin uuids as a string property on the assembly,
  // then for each pin op would store its layout as an int property.
  smtk::attribute::GroupItemPtr piecesGItem = op->findGroup("pins and layouts");
  size_t numPins;
  if (piecesGItem != nullptr)
  {
    smtk::model::StringList pinIds;
    numPins = piecesGItem->numberOfGroups();
    for (std::size_t index = 0; index < numPins; index++)
    {
      smtk::attribute::StringItemPtr pinIdItem =
        piecesGItem->findAs<smtk::attribute::StringItem>(index, "pin UUID");
      smtk::attribute::IntItemPtr schemaPlanItem =
        piecesGItem->findAs<smtk::attribute::IntItem>(index, "schema plan");

      std::string uuid = pinIdItem->value();
      pinIds.push_back(uuid);
      smtk::model::IntegerList layout(schemaPlanItem->begin(), schemaPlanItem->end());
      /***************************************************/
      std::cout << "  uuid=" << uuid << " with layouts as:\n    ";
      for (const auto& item : layout)
      {
        std::cout << item << " ";
      }
      std::cout << std::endl;
      /***************************************************/
      assembly.setIntegerProperty(uuid, layout);
    }
    assembly.setStringProperty("pins", pinIds);
    /******************************************************/
    std::cout << "  pins:\n";
    for (const auto& pinId : pinIds)
    {
      std::cout << "    " << pinId;
    }
    std::cout << std::endl;
    /******************************************************/
  }
  else
  {
    smtkErrorMacro(
      op->log(), "assembly " << assName << " does not have pins and their layouts info");
  }

  smtk::attribute::ModelEntityItemPtr ductIdItem = op->findModelEntity("associated duct");
  if (ductIdItem && !createMode && (ductIdItem->numberOfValues() == 1))
  {
    assembly.setStringProperty("associated duct", ductIdItem->value().entity().toString());
    /******************************************************/
    std::cout << "  duct=" << ductIdItem->value().entity().toString() << std::endl;
    /******************************************************/
  }

  smtk::attribute::VoidItemPtr isCenteredPinsItem = op->findVoid("center pins");
  if (isCenteredPinsItem->isEnabled())
  {
    assembly.setIntegerProperty(isCenteredPinsItem->name(), 1);
  }
  else
  {
    assembly.setIntegerProperty(isCenteredPinsItem->name(), 0);
  }
  /******************************************************/
  std::cout << "  centerPins=" << isCenteredPinsItem->isEnabled() << std::endl;
  /******************************************************/

  smtk::attribute::DoubleItemPtr pitchesItem = op->findDouble("pitches");
  if (pitchesItem)
  {
    if (pitchesItem->numberOfValues() == 1)
    { // Default value condition
      smtk::model::FloatList pitches{ pitchesItem->value(), pitchesItem->value() };
      assembly.setFloatProperty(pitchesItem->name(), pitches);
      /******************************************************/
      std::cout << "  pitches=" << pitches[0] << " " << pitches[1] << std::endl;
      /******************************************************/
    }
    else if (pitchesItem->numberOfValues() == 2)
    {
      smtk::model::FloatList pitches(pitchesItem->begin(), pitchesItem->end());
      assembly.setFloatProperty(pitchesItem->name(), pitches);
      /******************************************************/
      std::cout << "  pitches=" << pitches[0] << " " << pitches[1] << std::endl;
      /******************************************************/
    }
    else
    {
      smtkErrorMacro(op->log(), "assembly " << assName << " does not have valid pitch values");
    }
  }

  smtk::attribute::IntItemPtr latticeSizeItem = op->findInt("lattice size");
  if (latticeSizeItem)
  {
    if (latticeSizeItem->numberOfValues() == 1)
    { // Default value condition
      int unitSize = (isHex->isEnabled()) ? 1 : 4;
      smtk::model::IntegerList size = { unitSize, unitSize };
      assembly.setIntegerProperty(latticeSizeItem->name(), size);
      /******************************************************/
      std::cout << "  latticeSize=" << size[0] << " " << size[1] << std::endl;
      /******************************************************/
    }
    else if (latticeSizeItem->numberOfValues() == 2 && !createMode)
    { // Edit assembly
      smtk::model::IntegerList size(latticeSizeItem->begin(), latticeSizeItem->end());
      assembly.setIntegerProperty(latticeSizeItem->name(), size);
      /******************************************************/
      std::cout << "  latticeSize=" << size[0] << " " << size[1] << std::endl;
      /******************************************************/
    }
    else
    {
      smtkErrorMacro(op->log(), "assembly " << assName << " does not have a valid lattice size");
    }
  }

  smtk::attribute::IntItemPtr zAxisItem = op->findInt("z axis");
  if (zAxisItem)
  {
    assembly.setIntegerProperty(zAxisItem->name(), zAxisItem->value());
    smtkWarningMacro(op->log(), "For now, SMTK assembly does not support z axis value yet");
  }
  else
  {
    smtkErrorMacro(op->log(), "assembly " << assName << " does not have a valid z axis value");
  }
}

} // namespace rgg
} //namespace bridge
} // namespace smtk

smtkImplementsModelOperator(SMTKRGGSESSION_EXPORT, smtk::bridge::rgg::CreateAssembly,
  rgg_create_assembly, "create assembly", CreateAssembly_xml, smtk::bridge::rgg::Session);
