//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/bridge/rgg/operators/CreateDuct.h"

#include "smtk/bridge/rgg/Session.h"

#include "smtk/io/Logger.h"

#include "smtk/PublicPointerDefs.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/attribute/VoidItem.h"
#include "smtk/model/AuxiliaryGeometry.h"
#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/Operator.h"

#include "smtk/bridge/rgg/CreateDuct_xml.h"

#include <string> // std::to_string
using namespace smtk::model;

namespace smtk
{
namespace bridge
{
namespace rgg
{

smtk::model::OperatorResult CreateDuct::operateInternal()
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
    smtkErrorMacro(this->log(), "An invalid model is provided for create duct op");
    return this->createResult(smtk::operation::Operator::OPERATION_FAILED);
  }
  EntityRef parent = entities[0];

  smtk::model::AuxiliaryGeometry auxGeom;
  // A list contains all segments and layers of the duct
  std::vector<EntityRef> subAuxGeoms;

  auxGeom = parent.manager()->addAuxiliaryGeometry(parent.as<smtk::model::Model>(), 3);
  CreateDuct::populateDuct(dynamic_cast<smtk::model::Operator*>(this), auxGeom, subAuxGeoms);

  result = this->createResult(smtk::operation::Operator::OPERATION_SUCCEEDED);
  std::cout << "Add " << subAuxGeoms.size() << " subAuxGeoms to the duct" << std::endl;

  this->addEntityToResult(result, auxGeom, CREATED);
  this->addEntitiesToResult(result, subAuxGeoms, CREATED);
  return result;
}

void CreateDuct::populateDuct(
  smtk::model::Operator* op, AuxiliaryGeometry& auxGeom, std::vector<EntityRef>& subAuxGeoms)
{
  auxGeom.setStringProperty("rggType", SMTK_BRIDGE_RGG_DUCT);

  smtk::attribute::StringItemPtr nameItem = op->findString("name");
  std::string ductName;
  if (nameItem != nullptr && !nameItem->value(0).empty())
  {
    ductName = nameItem->value(0);
    auxGeom.setName(nameItem->value(0));
  }
  else
  {
    smtkErrorMacro(op->log(), " Fail to set name on the rgg duct");
  }

  // FIXME: It should be specified at core level
  // Hex or rectilinear
  smtk::attribute::VoidItemPtr isHex = op->findVoid("hex");
  if (isHex->isEnabled())
  {
    auxGeom.setIntegerProperty("hex", 1);
  }
  else
  {
    auxGeom.setIntegerProperty("hex", 0);
  }

  // Cut away the duct?
  smtk::attribute::VoidItemPtr isCrossSection = op->findVoid("cross section");
  if (isCrossSection->isEnabled())
  {
    auxGeom.setIntegerProperty("cross section", 1);
  }
  else
  {
    auxGeom.setIntegerProperty("cross section", 0);
  }

  // FIXME: It should be specified at core level
  // Set pitch
  smtk::attribute::DoubleItemPtr pitchItemItem = op->findDouble("pitch");
  if (pitchItemItem != nullptr && pitchItemItem->numberOfValues() == 2)
  { // Hex duct has two same pitch values and Rectilinear duct has two values
    smtk::model::FloatList tmp = { pitchItemItem->value(0), pitchItemItem->value(1) };
    auxGeom.setFloatProperty(pitchItemItem->name(), tmp);
  }
  else
  {
    smtkErrorMacro(op->log(), " Fail to set pitch on the rgg duct");
  }

  // FIXME: It should be specified at core level
  smtk::attribute::DoubleItemPtr ductHeightI = op->findDouble("duct height");
  if (ductHeightI != nullptr && ductHeightI->numberOfValues() == 2)
  {
    smtk::model::FloatList tmp = { ductHeightI->value(0), ductHeightI->value(1) };
    auxGeom.setFloatProperty(ductHeightI->name(), tmp);
  }
  else
  {
    smtkErrorMacro(op->log(), " Fail to set duct height on the rgg duct");
  }

  smtk::attribute::GroupItemPtr segsGItem = op->findGroup("duct segments");
  size_t numSegs;
  smtk::model::IntegerList numMaterialsPerSeg;
  if (segsGItem != nullptr)
  {
    numSegs = segsGItem->numberOfGroups();
    IntegerList materials;
    FloatList zValues, thicknesses;
    for (std::size_t index = 0; index < numSegs; index++)
    {
      // each duct segment has 2 z values, n(n >=1) materials and 2n thicknesses
      smtk::attribute::DoubleItemPtr zValuesI =
        segsGItem->findAs<smtk::attribute::DoubleItem>(index, "z values");
      smtk::attribute::IntItemPtr materialsI =
        segsGItem->findAs<smtk::attribute::IntItem>(index, "materials");
      smtk::attribute::DoubleItemPtr thicknessesI =
        segsGItem->findAs<smtk::attribute::DoubleItem>(index, "thicknesses(normalized)");
      // Cache number of materials of current seg
      numMaterialsPerSeg.push_back(materialsI->numberOfValues());
      // User is allowed to change the duct height in creating status,
      // so instead of querying zValuesI here we should query ductHeight
      if (zValuesI->numberOfValues() == 2 && (zValuesI->value(0) == 0) && (zValuesI->value(1) == 0))
      { // Initial invalid value, so use the duct height instead
        zValues.insert(zValues.end(), ductHeightI->begin(), ductHeightI->end());
      }
      else
      {
        zValues.insert(zValues.end(), zValuesI->begin(), zValuesI->end());
      }
      materials.insert(materials.end(), materialsI->begin(), materialsI->end());

      thicknesses.insert(thicknesses.end(), thicknessesI->begin(), thicknessesI->end());
      // FIXME: It should be provided by the core, remove the below line in the future
      // Since xml parser cannot handle a valueItem with 2 required&default values
      // and being extensible properly, we add the default value twice
      // Ideally if it's hex, it has two same values. If not, it should have
      // one thickness along width and another along height
      if (thicknessesI->numberOfValues() == 1)
      {
        thicknesses.insert(thicknesses.end(), thicknessesI->begin(), thicknessesI->end());
      }
    }
    auxGeom.setFloatProperty("z values", zValues);
    auxGeom.setIntegerProperty("materials", materials);
    auxGeom.setFloatProperty("thicknesses(normalized)", thicknesses);
  }
  // Helper property for segments which would be used as offset hint
  auxGeom.setIntegerProperty("material nums per segment", numMaterialsPerSeg);

  // Create auxgeom placeholders for layers and parts
  for (std::size_t i = 0; i < numSegs; i++)
  {
    for (std::size_t j = 0; j < numMaterialsPerSeg[i]; j++)
    {
      // Create an auxo_geom for every layer in current segment
      AuxiliaryGeometry subLayer = auxGeom.manager()->addAuxiliaryGeometry(auxGeom, 3);
      std::string subLName = ductName + SMTK_BRIDGE_RGG_DUCT_SEGMENT + std::to_string(i) +
        SMTK_BRIDGE_RGG_DUCT_LAYER + std::to_string(j);
      subLayer.setName(subLName);
      subLayer.setStringProperty("rggType", SMTK_BRIDGE_RGG_DUCT);
      subAuxGeoms.push_back(subLayer.as<EntityRef>());
    }
  }
}

} // namespace rgg
} //namespace bridge
} // namespace smtk

smtkImplementsModelOperator(SMTKRGGSESSION_EXPORT, smtk::bridge::rgg::CreateDuct, rgg_create_duct,
  "create duct", CreateDuct_xml, smtk::bridge::rgg::Session);
