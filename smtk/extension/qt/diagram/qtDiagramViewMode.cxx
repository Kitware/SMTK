//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/diagram/qtDiagramViewMode.h"

#include "smtk/extension/qt/GroupOps.h"
#include "smtk/extension/qt/diagram/qtBaseObjectNode.h"
#include "smtk/extension/qt/diagram/qtDiagram.h"
#include "smtk/extension/qt/diagram/qtDiagramScene.h"
#include "smtk/extension/qt/diagram/qtDiagramViewConfiguration.h"

#include "smtk/io/Logger.h"

#include <QGraphicsItem>

namespace smtk
{
namespace extension
{

qtDiagramViewMode::qtDiagramViewMode(
  smtk::string::Token modeName,
  qtDiagram* diagram,
  QToolBar* toolbar,
  QActionGroup* modeGroup)
  : Superclass(modeGroup)
  , m_modeName(modeName)
  , m_diagram(diagram)
{
  auto qModeName = QString::fromStdString(modeName.data());
  this->setObjectName(qModeName + " mode");
  m_modeAction = modeGroup->addAction(qModeName);
  m_modeAction->setCheckable(true);
  m_modeAction->setObjectName(qModeName);

  toolbar->addAction(m_modeAction);
}

qtDiagramViewMode::~qtDiagramViewMode() = default;

bool qtDiagramViewMode::isModeActive() const
{
  return m_modeAction->isChecked();
}

bool qtDiagramViewMode::removeSelectedObjects()
{
  if (!m_diagram)
  {
    return false;
  }
  auto operationManager = m_diagram->managers()->get<smtk::operation::Manager::Ptr>();
  if (!operationManager)
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(), "Cannot delete arcs without an operation manager.");
    return false;
  }

  std::set<std::shared_ptr<smtk::resource::PersistentObject>> objects;
  for (auto* item : m_diagram->diagramScene()->selectedItems())
  {
    if (auto* node = dynamic_cast<qtBaseObjectNode*>(item))
    {
      if (node->object())
      {
        objects.insert(node->object()->shared_from_this());
      }
    }
  }
  bool didLaunch = smtk::operation::deleteObjects(
    objects, operationManager, smtk::extension::qtDeleterDisposition);
  return didLaunch;
}

} // namespace extension
} // namespace smtk
