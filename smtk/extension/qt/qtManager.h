//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_qtManager_h
#define smtk_extension_qtManager_h

#include "smtk/extension/qt/diagram/qtDiagramGeneratorFactory.h"
#include "smtk/extension/qt/diagram/qtDiagramViewModeFactory.h"
#include "smtk/extension/qt/diagram/qtObjectNodeFactory.h"
#include "smtk/extension/qt/diagram/qtTaskNodeFactory.h"

#include "smtk/common/Deprecation.h"

namespace smtk
{
namespace extension
{

///\brief A qtManager is responsible for creating new qt-based objects.
///
/// Currently it provides a factory for creating qtTaskNodes, qtBaseObjectNodes,
/// qtDiagramGenerators, and qtDiagramViewModes.
class SMTKQTEXT_EXPORT qtManager : smtkEnableSharedPtr(qtManager)
{
public:
  smtkTypeMacroBase(smtk::extension::qtManager);
  smtkCreateMacro(smtk::extension::qtManager);

  virtual ~qtManager() = default;

  /// A factory to create instances of named subclasses of qtDiagramGenerator.
  qtDiagramGeneratorFactory& diagramGeneratorFactory() { return m_diagramGeneratorFactory; }
  const qtDiagramGeneratorFactory& diagramGeneratorFactory() const
  {
    return m_diagramGeneratorFactory;
  }

  /// A factory to create instances of named subclasses of qtDiagramViewMode.
  qtDiagramViewModeFactory& diagramViewModeFactory() { return m_diagramViewModeFactory; }
  const qtDiagramViewModeFactory& diagramViewModeFactory() const
  {
    return m_diagramViewModeFactory;
  }

  /// A factory to create object nodes from persistent objects.
  qtObjectNodeFactory& objectNodeFactory() { return m_objectNodeFactory; }
  const qtObjectNodeFactory& objectNodeFactory() const { return m_objectNodeFactory; }

  /// A factory to create task nodes from tasks.
  qtTaskNodeFactory& taskNodeFactory() { return m_taskNodeFactory; }
  const qtTaskNodeFactory& taskNodeFactory() const { return m_taskNodeFactory; }

private:
  qtDiagramGeneratorFactory m_diagramGeneratorFactory;
  qtDiagramViewModeFactory m_diagramViewModeFactory;
  qtObjectNodeFactory m_objectNodeFactory;
  qtTaskNodeFactory m_taskNodeFactory;

protected:
  qtManager() = default;
};
} // namespace extension
} // namespace smtk
#endif
