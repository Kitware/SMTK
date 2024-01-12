//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/task/qtGraphViewMode.h"

#include "smtk/extension/qt/GroupOps.h"
#include "smtk/extension/qt/task/qtBaseTaskNode.h"
#include "smtk/extension/qt/task/qtTaskEditor.h"
#include "smtk/extension/qt/task/qtTaskScene.h"
#include "smtk/extension/qt/task/qtTaskViewConfiguration.h"
#include "smtk/io/Logger.h"

#include <QGraphicsItem>

namespace smtk
{
namespace extension
{

qtGraphViewMode::qtGraphViewMode(
  smtk::string::Token modeName,
  qtTaskEditor* editor,
  QToolBar* toolbar,
  QActionGroup* modeGroup)
  : Superclass(modeGroup)
  , m_modeName(modeName)
  , m_editor(editor)
{
  auto qModeName = QString::fromStdString(modeName.data());
  this->setObjectName(qModeName + " mode");
  m_modeAction = modeGroup->addAction(qModeName);
  m_modeAction->setCheckable(true);
  m_modeAction->setObjectName(qModeName);

  toolbar->addAction(m_modeAction);
}

qtGraphViewMode::~qtGraphViewMode() = default;

bool qtGraphViewMode::isModeActive() const
{
  return m_modeAction->isChecked();
}

bool qtGraphViewMode::removeSelectedObjects()
{
  if (!m_editor)
  {
    return false;
  }
  auto operationManager = m_editor->managers()->get<smtk::operation::Manager::Ptr>();
  if (!operationManager)
  {
    smtkErrorMacro(
      smtk::io::Logger::instance(), "Cannot delete arcs without an operation manager.");
    return false;
  }

  std::set<std::shared_ptr<smtk::resource::PersistentObject>> objects;
  for (auto* item : m_editor->taskScene()->selectedItems())
  {
    if (auto* node = dynamic_cast<qtBaseTaskNode*>(item))
    {
      if (node->task())
      {
        objects.insert(node->task()->shared_from_this());
      }
    }
  }
  bool didLaunch = smtk::operation::deleteObjects(
    objects, operationManager, smtk::extension::qtDeleterDisposition);
  return didLaunch;
}

} // namespace extension
} // namespace smtk
