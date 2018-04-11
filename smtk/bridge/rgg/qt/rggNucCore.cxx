//=============================================================================
// Copyright (c) Kitware, Inc.
// All rights reserved.
// See LICENSE.txt for details.
//
// This software is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE.  See the above copyright notice for more information.
//=============================================================================
#include "smtk/bridge/rgg/qt/rggNucCore.h"
#include "smtk/bridge/rgg/qt/qtLattice.h"

#include "smtk/bridge/rgg/operators/CreateAssembly.h"
#include "smtk/extension/qt/qtActiveObjects.h"

#include "smtk/io/Logger.h"
#include "smtk/model/AuxiliaryGeometry.h"
#include "smtk/model/Group.h"
#include "smtk/model/Manager.h"

rggNucCore::rggNucCore(smtk::model::EntityRef entity)
  : rggLatticeContainer(entity)
{
  this->resetBySMTKCore(entity);
}

rggNucCore::~rggNucCore()
{
}

QString rggNucCore::extractLabel(QString const& el)
{ // Extract the label out of context menu action name
  QString seperator("(");
  QStringList ql = el.split(seperator);
  return ql[1].left(ql[1].size() - 1);
}

void rggNucCore::fillList(std::vector<std::pair<QString, smtk::model::EntityRef> >& l)
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
  smtk::model::EntityRefArray assies =
    ptr->findEntitiesByProperty("rggType", SMTK_BRIDGE_RGG_ASSEMBLY);
  // Add all available pins that match the assembly's geometry type
  for (auto& assy : assies)
  {
    if (assy.as<smtk::model::Group>().isValid())
    {
      l.push_back(generatePair(assy));
    }
  }
}

smtk::model::EntityRef rggNucCore::getFromLabel(const QString& label)
{
  smtk::model::ManagerPtr ptr = qtActiveObjects::instance().activeModel().manager();
  smtk::model::EntityRefArray assies = ptr->findEntitiesByProperty("label", label.toStdString());
  for (auto& assy : assies)
  {
    if (assy.hasStringProperty("rggType") &&
      assy.stringProperty("rggType")[0] == SMTK_BRIDGE_RGG_ASSEMBLY)
    { // Each assy has a unique label
      return assy;
    }
  }
  return smtk::model::EntityRef();
}

bool rggNucCore::IsHexType()
{
  return this->m_lattice.GetGeometryType() == HEXAGONAL;
}

void rggNucCore::calculateExtraTranslation(double& transX, double& transY)
{
  // "TODO: rggNucCore::calculateExtraTranslation"
}

void rggNucCore::calculateTranslation(double& transX, double& transY)
{
  // "TODO: rggNucCore::calculateTranslation"
}

void rggNucCore::setUpdateUsed()
{
  // "TODO: rggNucCore::setUpdateUsed"
}

void rggNucCore::getRadius(double& ri, double& rj) const
{
  // "TODO: rggNucCore::getRadius"
}

void rggNucCore::resetBySMTKCore(const smtk::model::EntityRef& core)
{
  this->m_entity = core;
  smtk::model::Model model = core.owningModel();
  smtk::model::ManagerPtr mgr = model.manager();

  if (!core.isGroup())
  {
    smtkErrorMacro(smtk::io::Logger(), "A non group type core is passed"
                                       " into rggNucCore!");
    return;
  }

  // Update lattice related info
  if (model.hasIntegerProperty("hex"))
  {
    int isHex = model.integerProperty("hex")[0];
    this->m_lattice.SetGeometryType(
      isHex ? rggGeometryType::HEXAGONAL : rggGeometryType::RECTILINEAR);
  }
  else
  {
    smtkErrorMacro(smtk::io::Logger(), "The core does not have a valid hex info");
  }

  if (model.hasIntegerProperty("lattice size") && model.integerProperty("lattice size").size() == 2)
  {
    smtk::model::IntegerList lSize = model.integerProperty("lattice size");
    this->m_lattice.SetDimensions(lSize[0], lSize[1]);
  }
  else
  {
    smtkErrorMacro(smtk::io::Logger(), "The core does not have a valid lattice"
                                       " size");
  }

  // Populate the qtLattice
  if (model.hasStringProperty("assemblies"))
  {
    smtk::model::StringList assyIds = model.stringProperty("assemblies");
    for (auto& assyId : assyIds)
    {
      smtk::model::EntityRef assy = smtk::model::EntityRef(mgr, assyId);
      if (!assy.isValid())
      {
        smtkErrorMacro(smtk::io::Logger(), "Core's assy ID " << assyId << " is not valid");
        continue;
      }
      if (!model.hasIntegerProperty(assyId))
      {
        smtkErrorMacro(
          smtk::io::Logger(), "Assembly's assy " << assy.name() << " does not have layout info");
        continue;
      }
      smtk::model::IntegerList layout = model.integerProperty(assyId);
      size_t numOfPair = layout.size() / 2;
      for (size_t i = 0; i < numOfPair; i++)
      {
        this->m_lattice.SetCell(layout[2 * i], layout[2 * i + 1], assy);
      }
    }
  }
  else
  {
    smtkErrorMacro(smtk::io::Logger(), "Assembly does not have assys associated.");
  }
}
