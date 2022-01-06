//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================
#include "smtk/extension/paraview/widgets/pqCrinklePropertyWidget.h"
#include "smtk/extension/paraview/widgets/pqSlicePropertyWidgetBaseP.h"

#include "vtkSMIntVectorProperty.h"
#include "vtkSMNewWidgetRepresentationProxy.h"
#include "vtkSMPropertyGroup.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMRenderViewProxy.h"

#include <QCheckBox>

pqCrinklePropertyWidget::pqCrinklePropertyWidget(
  pqPipelineSource* input,
  vtkSMProxy* smproxy,
  vtkSMPropertyGroup* smgroup,
  QWidget* parentObject)
  : Superclass(
      "representations",
      "MeshInspectorWidgetRepresentation",
      input,
      smproxy,
      smgroup,
      parentObject)
{
  vtkSMPropertyHelper(this->widgetProxy(), "SliceType").Set("Crinkle");

  // We want to show the translucent plane when interaction starts.
  // The base class does not do this by default because when slicing
  // images (as pqSlicePropertyWidget does), this produces z-fighting.
  this->connect(this, SIGNAL(startInteraction()), SLOT(showPlane()));
}

pqCrinklePropertyWidget::~pqCrinklePropertyWidget() = default;
