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

#include "smtk/bridge/rgg/operators/CreateModel.h"

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
  // Update the latest duct in the model
  parent.setStringProperty("latest duct", auxGeom.entity().toString());

  result = this->createResult(smtk::operation::Operator::OPERATION_SUCCEEDED);

  this->addEntityToResult(result, parent, MODIFIED);
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

  smtk::attribute::GroupItemPtr segsGItem = op->findGroup("duct segments");
  size_t numSegs;
  smtk::model::IntegerList numMaterialsPerSeg;
  IntegerList materials;
  if (segsGItem != nullptr)
  {
    numSegs = segsGItem->numberOfGroups();
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
      // When creating duct, we just use the duct height stored on the core
      // so instead of querying zValuesI here we should query the ductHeight
      if (zValuesI->numberOfValues() == 2 && (zValuesI->value(0) == 0) && (zValuesI->value(1) == 0))
      { // Initial invalid value, so use the duct height instead
        smtk::model::FloatList ductHeight = auxGeom.owningModel().floatProperty("duct height");
        zValues.insert(zValues.end(), ductHeight.begin(), ductHeight.end());
      }
      else
      {
        zValues.insert(zValues.end(), zValuesI->begin(), zValuesI->end());
      }
      materials.insert(materials.end(), materialsI->begin(), materialsI->end());

      thicknesses.insert(thicknesses.end(), thicknessesI->begin(), thicknessesI->end());
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
  // Helper property for segments which would be used as an offset hint
  auxGeom.setIntegerProperty("material nums per segment", numMaterialsPerSeg);

  auto assignColor = [](size_t index, smtk::model::AuxiliaryGeometry& aux) {
    smtk::model::FloatList rgba;
    smtk::bridge::rgg::CreateModel::getMaterialColor(index, rgba, aux.owningModel());
    aux.setColor(rgba);
  };

  // Create auxgeom placeholders for layers and parts
  size_t materialIndex(0);
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
      assignColor(materials[materialIndex++], subLayer);
      subAuxGeoms.push_back(subLayer.as<EntityRef>());
    }
  }
}

} // namespace rgg
} //namespace bridge
} // namespace smtk

smtkImplementsModelOperator(SMTKRGGSESSION_EXPORT, smtk::bridge::rgg::CreateDuct, rgg_create_duct,
  "create duct", CreateDuct_xml, smtk::bridge::rgg::Session);
