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

} // namespace extension
} // namespace smtk
