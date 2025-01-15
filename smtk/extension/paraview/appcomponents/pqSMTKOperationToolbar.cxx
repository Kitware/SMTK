//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/pqSMTKOperationToolbar.h"

#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKOperationToolboxPanel.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKWrapper.h"
#include "smtk/extension/paraview/server/vtkSMSMTKWrapperProxy.h"
#include "smtk/extension/qt/qtOperationAction.h"
#include "smtk/extension/qt/qtOperationPalette.h"
#include "smtk/extension/qt/qtOperationTypeModel.h"

// ParaView
#include "pqApplicationCore.h"

// Qt
#include <QAction>
#include <QLabel>
#include <QTimer>

// C++
#include <tuple>
#include <type_traits>
#include <typeinfo>
#include <utility>

class pqSMTKOperationToolbar::pqInternal
{
public:
  pqInternal(pqSMTKOperationToolbar* toolbar)
    : m_toolbar(toolbar)
  {
  }

  void populateToolbar()
  {
    // Because populateToolbar is called after the event loop starts
    // (and thus after plugins have loaded), the toolbox should have
    // a valid qtOperationTypeModel for us.
    auto* core = pqApplicationCore::instance();
    auto* panel =
      qobject_cast<pqSMTKOperationToolboxPanel*>(core->manager("smtk operation toolbox"));
    if (!panel)
    {
      smtkErrorMacro(smtk::io::Logger::instance(), "No toolbox panel (needed for toolbar).");
      return;
    }
    auto toolbox = panel->toolbox();
    if (!toolbox)
    {
      smtkErrorMacro(smtk::io::Logger::instance(), "No toolbox (needed for toolbar).");
      return;
    }
    auto* model = toolbox->operationModel();
    if (model && m_toolbar)
    {
      m_toolbar->populateToolbar(model);
    }
  }

  pqSMTKOperationToolbar* m_toolbar{ nullptr };
};

pqSMTKOperationToolbar::pqSMTKOperationToolbar(QWidget* parent)
  : Superclass(parent)
{
  m_p = new pqInternal(this);

  // Wait until the event loop has started to populate the toolbar
  // so that the toolbox panel is guaranteed to exist and have a model
  // we can reference.
  QTimer::singleShot(0, [this]() { m_p->populateToolbar(); });
}

pqSMTKOperationToolbar::~pqSMTKOperationToolbar()
{
  delete m_p;
}
