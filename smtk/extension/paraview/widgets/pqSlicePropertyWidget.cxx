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
#include "smtk/extension/paraview/widgets/pqSlicePropertyWidget.h"
#include "smtk/extension/paraview/widgets/pqSlicePropertyWidgetBaseP.h"

#include "vtkSMIntVectorProperty.h"
#include "vtkSMNewWidgetRepresentationProxy.h"
#include "vtkSMPropertyGroup.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMRenderViewProxy.h"

#include "vtkDataObject.h"

#include <QCheckBox>

pqSlicePropertyWidget::pqSlicePropertyWidget(
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
  // Continue configuring defaults.
  // Flat slices are mostly for image data where drawing
  // edges would obscure the scalar color visibility.
  m_p->drawCrinkleEdges->setCheckState(Qt::Unchecked);

  vtkSMProxy* wdgProxy = this->widgetProxy();
  // wdgProxy->UpdatePropertyInformation();
  // TODO: Look up field assoc and array name for associated Volume cell:
  vtkSMPropertyHelper(wdgProxy, "SliceColorArray")
    .SetInputArrayToProcess(vtkDataObject::POINT, "scalars");
}

pqSlicePropertyWidget::~pqSlicePropertyWidget() = default;
