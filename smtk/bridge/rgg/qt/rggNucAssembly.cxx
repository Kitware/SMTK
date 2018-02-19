//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/bridge/rgg/qt/rggNucAssembly.h"
#include "smtk/bridge/rgg/qt/qtLattice.h"

#include "smtk/bridge/rgg/operators/CreatePin.h"
#include "smtk/extension/qt/qtActiveObjects.h"

#include "smtk/io/Logger.h"
#include "smtk/model/AuxiliaryGeometry.h"
#include "smtk/model/Manager.h"

rggNucAssembly::rggNucAssembly(smtk::model::EntityRef entity)
  : rggLatticeContainer(entity)
  , m_zAxisRotation(0)
  , m_centerPins(true)
{
  this->resetBySMTKAssembly(entity);
}

rggNucAssembly::~rggNucAssembly()
{
}

QString rggNucAssembly::extractLabel(QString const& el)
{ // Extract the label out of context menu action name
  QString seperator("(");
  QStringList ql = el.split(seperator);
  return ql[1].left(ql[1].size() - 1);
}

void rggNucAssembly::fillList(std::vector<std::pair<QString, smtk::model::EntityRef> >& l)
{
  auto generatePair = [](smtk::model::EntityRef& entity) {
    std::string newName;
    if (entity.hasStringProperty("label"))
    {
      newName = entity.name() + " (" + entity.stringProperty("label")[0] + ")";
    }
    else
    {
      newName = entity.name() + " ()";
    }
    return std::pair<QString, smtk::model::EntityRef>(QString::fromStdString(newName), entity);
  };

  smtk::model::ManagerPtr ptr = qtActiveObjects::instance().activeModel().manager();
  // Add empty cell first
  if (ptr)
  {
    if (ptr->findEntitiesByProperty("label", "XX").size() > 0)
    {
      l.push_back(generatePair(ptr->findEntitiesByProperty("label", "XX")[0]));
    }
  }
  smtk::model::EntityRefArray pins = ptr->findEntitiesByProperty("rggType", SMTK_BRIDGE_RGG_PIN);
  // Add all available pins that match the assembly's geometry type
  for (auto& pin : pins)
  {
    if (pin.as<smtk::model::AuxiliaryGeometry>().auxiliaryGeometries().size() > 0)
    { // Do not add sub pin parts and layers
      l.push_back(generatePair(pin));
    }
  }
}

smtk::model::EntityRef rggNucAssembly::getFromLabel(const QString& label)
{
  smtk::model::ManagerPtr ptr = qtActiveObjects::instance().activeModel().manager();
  smtk::model::EntityRefArray pins = ptr->findEntitiesByProperty("label", label.toStdString());
  for (auto& pin : pins)
  {
    if (pin.hasStringProperty("rggType") && pin.stringProperty("rggType")[0] == SMTK_BRIDGE_RGG_PIN)
    { // Each pin has a unique label
      return pin;
    }
  }
  return smtk::model::EntityRef();
}

bool rggNucAssembly::IsHexType()
{
  return this->m_lattice.GetGeometryType() == HEXAGONAL;
}

void rggNucAssembly::calculateExtraTranslation(double& transX, double& transY)
{
  std::cout << "TODO: rggNucAssembly::calculateExtraTranslation" << std::endl;
}

void rggNucAssembly::calculateTranslation(double& transX, double& transY)
{
  std::cout << "TODO: rggNucAssembly::calculateTranslation" << std::endl;
}

void rggNucAssembly::setUpdateUsed()
{
  std::cout << "TODO: rggNucAssembly::setUpdateUsed" << std::endl;
}

void rggNucAssembly::getRadius(double& ri, double& rj) const
{
  std::cout << "TODO: rggNucAssembly::getRadius" << std::endl;
}

void rggNucAssembly::resetBySMTKAssembly(const smtk::model::EntityRef& assy)
{
  this->m_entity = assy;

  if (assy.hasIntegerProperty("z axis"))
  {
    this->m_zAxisRotation = assy.integerProperty("z axis")[0];
    std::cout << "  z axis=" << this->m_zAxisRotation << std::endl;
  }

  if (assy.hasIntegerProperty("center pins"))
  {
    this->m_centerPins = assy.integerProperty("center pins")[0];
    std::cout << "  center pins=" << this->m_centerPins << std::endl;
  }

  // Update lattice related info
  if (assy.hasIntegerProperty("hex"))
  {
    int isHex = assy.integerProperty("hex")[0];
    std::cout << "  hex=" << this->m_centerPins << std::endl;
    this->m_lattice.SetGeometryType(
      isHex ? rggGeometryType::HEXAGONAL : rggGeometryType::RECTILINEAR);
  }
  else
  {
    smtkErrorMacro(smtk::io::Logger(), "Assembly does not have valid hex info");
  }

  if (assy.hasIntegerProperty("lattice size") && assy.integerProperty("lattice size").size() == 2)
  {
    smtk::model::IntegerList lSize = assy.integerProperty("lattice size");
    std::cout << "  lattice size=" << lSize[0] << " " << lSize[1] << std::endl;
    this->m_lattice.SetDimensions(lSize[0], lSize[1]);
  }
  else
  {
    smtkErrorMacro(smtk::io::Logger(), "Assembly does not have a valid lattice"
                                       " size");
  }
  //TODO: handle pins, ducts and populate the qtLattice
}

void rggNucAssembly::setAssyDuct(smtk::model::EntityRef duct)
{
  this->m_duct = duct;
}

smtk::model::EntityRef const& rggNucAssembly::getAssyDuct() const
{
  return this->m_duct;
}

smtk::model::EntityRef& rggNucAssembly::getAssyDuct()
{
  return this->m_duct;
}

void rggNucAssembly::setZAxisRotation(int value)
{
  this->m_zAxisRotation = value;
}

int rggNucAssembly::getZAxisRotation() const
{
  return this->m_zAxisRotation;
}

void rggNucAssembly::setCenterPins(bool isCentered)
{
  this->m_centerPins = isCentered;
}
bool rggNucAssembly::isPinsCentered() const
{
  return m_centerPins;
}
