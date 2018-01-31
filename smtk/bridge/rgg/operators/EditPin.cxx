//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/bridge/rgg/operators/EditPin.h"
#include "smtk/bridge/rgg/operators/CreatePin.h"

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

#include "smtk/bridge/rgg/EditPin_xml.h"

#include <string> // std::to_string
using namespace smtk::model;

namespace smtk
{
namespace bridge
{
namespace rgg
{

smtk::model::OperatorResult EditPin::operateInternal()
{
  smtk::model::OperatorResult result =
    this->createResult(smtk::operation::Operator::OPERATION_FAILED);
  smtk::bridge::rgg::SessionPtr sess = this->activeSession();
  if (!sess)
  {
    return result;
  }
  EntityRefArray entities = this->associatedEntitiesAs<EntityRefArray>();
  if (entities.empty() || !entities[0].isAuxiliaryGeometry())
  {
    smtkErrorMacro(this->log(), "Cannot edit a non auxiliary geometry entity");
    return this->createResult(smtk::operation::Operator::OPERATION_FAILED);
  }

  smtk::model::EntityRefArray expunged, modified, tobeDeleted;

  smtk::model::AuxiliaryGeometry auxGeom = entities[0].as<AuxiliaryGeometry>();
  // Remove all current child auxiliary geometries first
  EntityRefArray children = auxGeom.embeddedEntities<EntityRefArray>();
  auxGeom.setIntegerProperty("previous children size", children.size());
  tobeDeleted.insert(tobeDeleted.end(), children.begin(), children.end());

  if (this->manager())
  {
    this->session()->manager()->deleteEntities(
      tobeDeleted, modified, expunged, this->m_debugLevel > 0);
  }
  // A list contains all subparts and layers of the pin
  std::vector<EntityRef> subAuxGeoms;
  auxGeom.setStringProperty("rggType", SMTK_BRIDGE_RGG_PIN);

  // Hex or rectinlinear
  smtk::attribute::VoidItemPtr isHex = this->findVoid("hex");
  if (isHex->isEnabled())
  {
    auxGeom.setIntegerProperty("hex", 1);
  }
  else
  {
    auxGeom.setIntegerProperty("hex", 0);
  }

  // Cut away the pin?
  smtk::attribute::VoidItemPtr isCutAway = this->findVoid("cut away");
  if (isCutAway->isEnabled())
  {
    auxGeom.setIntegerProperty("cut away", 1);
  }
  else
  {
    auxGeom.setIntegerProperty("cut away", 0);
  }

  smtk::attribute::StringItemPtr nameItem = this->findString("name");
  std::string pinName;
  if (nameItem != nullptr && !nameItem->value(0).empty())
  {
    pinName = nameItem->value(0);
    auxGeom.setName(nameItem->value(0));
  }

  smtk::attribute::StringItemPtr labelItem = this->findString("label");
  if (labelItem != nullptr && !labelItem->value(0).empty())
  {
    auxGeom.setStringProperty(labelItem->name(), labelItem->value(0));
  }

  smtk::attribute::IntItemPtr pinMaterialItem = this->findInt("cell material");
  bool isMaterialSet(false);
  if (pinMaterialItem != nullptr && pinMaterialItem->numberOfValues() == 1)
  {
    auxGeom.setIntegerProperty(pinMaterialItem->name(), pinMaterialItem->value(0));
    isMaterialSet = static_cast<bool>(pinMaterialItem->value(0) > 0);
  }

  smtk::attribute::DoubleItemPtr zOriginItem = this->findDouble("z origin");
  if (zOriginItem != nullptr && zOriginItem->numberOfValues() == 1)
  {
    auxGeom.setFloatProperty(zOriginItem->name(), zOriginItem->value(0));
  }

  smtk::attribute::GroupItemPtr piecesGItem = this->findGroup("pieces");
  size_t numParts;
  if (piecesGItem != nullptr)
  {
    numParts = piecesGItem->numberOfGroups();
    IntegerList pieceSegType;
    FloatList typeParas;
    for (std::size_t index = 0; index < numParts; index++)
    {
      smtk::attribute::IntItemPtr segmentType =
        piecesGItem->findAs<smtk::attribute::IntItem>(index, "segment type");
      // Length, base radius, top radius
      smtk::attribute::DoubleItemPtr typePara =
        piecesGItem->findAs<smtk::attribute::DoubleItem>(index, "type parameters");
      pieceSegType.insert(pieceSegType.end(), segmentType->begin(), segmentType->end());
      typeParas.insert(typeParas.end(), typePara->begin(), typePara->end());
    }
    auxGeom.setIntegerProperty(piecesGItem->name(), pieceSegType);
    auxGeom.setFloatProperty(piecesGItem->name(), typeParas);
    // Get the max radius and cache it for create assembly purpose
    double maxRadius(-1);
    for (size_t i = 0; i < typeParas.size() / 3; i++)
    {
      maxRadius = std::max(maxRadius, std::max(typeParas[i * 3 + 1], typeParas[i * 3 + 2]));
    }
    auxGeom.setFloatProperty("max radius", maxRadius);
  }

  smtk::attribute::GroupItemPtr layerMaterialsItem = this->findGroup("layer materials");
  size_t numLayers;
  if (layerMaterialsItem != nullptr)
  {
    numLayers = layerMaterialsItem->numberOfGroups();
    IntegerList subMaterials;
    FloatList radiusNs;
    for (std::size_t index = 0; index < numLayers; index++)
    {
      smtk::attribute::IntItemPtr subMaterial =
        layerMaterialsItem->findAs<smtk::attribute::IntItem>(index, "sub material");
      smtk::attribute::DoubleItemPtr radisuN =
        layerMaterialsItem->findAs<smtk::attribute::DoubleItem>(index, "radius(normalized)");
      subMaterials.insert(subMaterials.end(), subMaterial->begin(), subMaterial->end());
      radiusNs.insert(radiusNs.end(), radisuN->begin(), radisuN->end());
    }
    auxGeom.setIntegerProperty(layerMaterialsItem->name(), subMaterials);
    auxGeom.setFloatProperty(layerMaterialsItem->name(), radiusNs);
  }

  // Create auxgeom placeholders for layers and parts
  for (std::size_t i = 0; i < numParts; i++)
  {
    for (std::size_t j = 0; j < numLayers; j++)
    {
      // Create an auxo_geom for current each unit part&layer
      AuxiliaryGeometry subLayer = auxGeom.manager()->addAuxiliaryGeometry(auxGeom, 3);
      std::string subLName = pinName + SMTK_BRIDGE_RGG_PIN_SUBPART + std::to_string(i) +
        SMTK_BRIDGE_RGG_PIN_LAYER + std::to_string(j);
      subLayer.setName(subLName);
      subLayer.setStringProperty("rggType", SMTK_BRIDGE_RGG_PIN);
      subAuxGeoms.push_back(subLayer.as<EntityRef>());
    }
    if (isMaterialSet)
    { // Append a material layer after the last layer
      AuxiliaryGeometry materialLayer = auxGeom.manager()->addAuxiliaryGeometry(auxGeom, 3);
      std::string materialName =
        pinName + SMTK_BRIDGE_RGG_PIN_SUBPART + std::to_string(i) + SMTK_BRIDGE_RGG_PIN_MATERIAL;
      materialLayer.setName(materialName);
      materialLayer.setStringProperty("rggType", SMTK_BRIDGE_RGG_PIN);
      subAuxGeoms.push_back(materialLayer.as<EntityRef>());
    }
  }

  result = this->createResult(smtk::operation::Operator::OPERATION_SUCCEEDED);

  this->addEntityToResult(result, auxGeom, MODIFIED);
  this->addEntitiesToResult(result, subAuxGeoms, CREATED);
  this->addEntitiesToResult(result, modified, MODIFIED);
  this->addEntitiesToResult(result, expunged, EXPUNGED);
  return result;
}

} // namespace rgg
} //namespace bridge
} // namespace smtk

smtkImplementsModelOperator(SMTKRGGSESSION_EXPORT, smtk::bridge::rgg::EditPin, rgg_edit_pin,
  "edit pin", EditPin_xml, smtk::bridge::rgg::Session);
