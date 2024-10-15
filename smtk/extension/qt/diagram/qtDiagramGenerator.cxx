//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/diagram/qtDiagramGenerator.h"

#include "smtk/extension/qt/diagram/qtBaseArc.h"
#include "smtk/extension/qt/diagram/qtBaseNode.h"
#include "smtk/extension/qt/diagram/qtDiagram.h"
#include "smtk/extension/qt/diagram/qtDiagramViewMode.h"

using namespace smtk::string::literals;

namespace smtk
{
namespace extension
{

qtDiagramGenerator::qtDiagramGenerator(
  const smtk::view::Information& info,
  const smtk::view::Configuration::Component& config,
  qtDiagram* parent)
  : m_diagram(parent)
{
  (void)info;
  (void)config;
}

void qtDiagramGenerator::updateSceneNodes(
  std::unordered_set<smtk::resource::PersistentObject*>& created,
  std::unordered_set<smtk::resource::PersistentObject*>& modified,
  std::unordered_set<smtk::resource::PersistentObject*>& expunged,
  const smtk::operation::Operation& operation,
  const smtk::operation::Operation::Result& result)
{
  smtkErrorMacro(
    smtk::io::Logger::instance(),
    "qtDiagramGenerator::updateScene() is deprecated; split updateSceneNodes and updateSceneArcs.");
  this->updateSceneNodes(created, modified, expunged, operation, result);
}

void qtDiagramGenerator::sceneCleared()
{
  if (!m_diagram)
  {
    return;
  }

  for (const auto& modeEntry : m_diagram->modes())
  {
    modeEntry.second->sceneCleared(this);
  }
}

void qtDiagramGenerator::updateArcsOfSendingNodeRecursive()
{
  auto* node = dynamic_cast<qtBaseNode*>(this->sender());
  if (node)
  {
    this->updateArcsOfNodeRecursive(node);
  }
}

void qtDiagramGenerator::updateArcsOfNodeRecursive(qtBaseNode* node)
{
  // I. Collect affected arcs (this may visit the same arcs twice).
  std::unordered_set<qtBaseArc*> affectedArcs;
  this->addArcsOfNodeRecursive(node, affectedArcs);
  // II. Update arc points (just once)
  for (const auto& arc : affectedArcs)
  {
    arc->updateArcPoints();
  }
}

void qtDiagramGenerator::addArcsOfNodeRecursive(
  qtBaseNode* node,
  std::unordered_set<qtBaseArc*>& arcs)
{
  auto selfArcs = m_diagram->arcsOfNode(node);
  arcs.insert(selfArcs.begin(), selfArcs.end());
  for (auto* child : node->childItems())
  {
    if (auto* childNode = dynamic_cast<qtBaseNode*>(child))
    {
      this->addArcsOfNodeRecursive(childNode, arcs);
    }
  }
}

} // namespace extension
} // namespace smtk
