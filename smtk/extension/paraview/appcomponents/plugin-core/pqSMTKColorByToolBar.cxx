//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/plugin-core/pqSMTKColorByToolBar.h"
#include "smtk/extension/paraview/appcomponents/plugin-core/pqSMTKColorByWidget.h"

// Client side
#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqServerManagerModel.h"
#include "pqSetName.h"

// Qt
#include <QAction>
#include <QToolBar>
#include <QWidget>

using namespace smtk;

pqSMTKColorByToolBar::pqSMTKColorByToolBar(QWidget* parent)
  : Superclass("SMTK Color Mode", parent)
{
  this->setObjectName("SMTKColorMode");
  auto* widget = new pqSMTKColorByWidget(this);
  widget << pqSetName("colorBy");
  widget->setToolTip("Color SMTK components by...");
  this->addWidget(widget);
  QObject::connect(
    &pqActiveObjects::instance(),
    SIGNAL(representationChanged(pqDataRepresentation*)),
    widget,
    SLOT(setRepresentation(pqDataRepresentation*)));

  // Now, if we find a sibling toolbar named "Active Variable Controls" hook it
  // up so it's only shown when the Color By mode is "Field" (or the active
  // representation is not an SMTK model):
  Q_FOREACH (QToolBar* avctb, parent->findChildren<QToolBar*>())
  {
    if (avctb->windowTitle() == "Active Variable Controls")
    {
      QObject::connect(widget, SIGNAL(colorByFieldActive(bool)), avctb, SLOT(setVisible(bool)));
    }
  }
}

pqSMTKColorByToolBar::~pqSMTKColorByToolBar() = default;
