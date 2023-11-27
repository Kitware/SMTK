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

#include "smtk/extension/qt/task/qtTaskEditor.h"
#include "smtk/extension/qt/task/qtTaskViewConfiguration.h"

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

} // namespace extension
} // namespace smtk
