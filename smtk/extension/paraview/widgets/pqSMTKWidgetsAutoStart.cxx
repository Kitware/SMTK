//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/widgets/pqSMTKWidgetsAutoStart.h"

#include "smtk/view/Selection.h"

#include "smtk/extension/paraview/widgets/pqSMTKBoxItemWidget.h"
#include "smtk/extension/paraview/widgets/pqSMTKConeItemWidget.h"
#include "smtk/extension/paraview/widgets/pqSMTKInfiniteCylinderItemWidget.h"
#include "smtk/extension/paraview/widgets/pqSMTKLineItemWidget.h"
#include "smtk/extension/paraview/widgets/pqSMTKPlaneItemWidget.h"
#include "smtk/extension/paraview/widgets/pqSMTKPointItemWidget.h"
#include "smtk/extension/paraview/widgets/pqSMTKSphereItemWidget.h"
#include "smtk/extension/paraview/widgets/pqSMTKSplineItemWidget.h"
#include "smtk/extension/paraview/widgets/qtSimpleExpressionEvaluationView.h"

#include "smtk/extension/qt/qtSMTKUtilities.h"

#include "pqApplicationCore.h"
#include "pqObjectBuilder.h"

pqSMTKWidgetsAutoStart::pqSMTKWidgetsAutoStart(QObject* parent)
  : Superclass(parent)
{
}

pqSMTKWidgetsAutoStart::~pqSMTKWidgetsAutoStart()
{
}

void pqSMTKWidgetsAutoStart::startup()
{
  /*
  auto pqCore = pqApplicationCore::instance();
  if (pqCore)
  {
  }
  */

  // Register qtItem widget subclasses implemented using ParaView 3-D widgets:
  qtSMTKUtilities::registerItemConstructor("Box", pqSMTKBoxItemWidget::createBoxItemWidget);
  qtSMTKUtilities::registerItemConstructor("Cone", pqSMTKConeItemWidget::createConeItemWidget);
  qtSMTKUtilities::registerItemConstructor(
    "Cylinder", pqSMTKConeItemWidget::createCylinderItemWidget);
  qtSMTKUtilities::registerItemConstructor(
    "InfiniteCylinder", pqSMTKInfiniteCylinderItemWidget::createCylinderItemWidget);
  qtSMTKUtilities::registerItemConstructor("Line", pqSMTKLineItemWidget::createLineItemWidget);
  qtSMTKUtilities::registerItemConstructor("Plane", pqSMTKPlaneItemWidget::createPlaneItemWidget);
  qtSMTKUtilities::registerItemConstructor("Point", pqSMTKPointItemWidget::createPointItemWidget);
  qtSMTKUtilities::registerItemConstructor(
    "Sphere", pqSMTKSphereItemWidget::createSphereItemWidget);
  qtSMTKUtilities::registerItemConstructor(
    "Spline", pqSMTKSplineItemWidget::createSplineItemWidget);
  qtSMTKUtilities::registerViewConstructor(
    "SimpleExpression", qtSimpleExpressionEvaluationView::createViewWidget);
}

void pqSMTKWidgetsAutoStart::shutdown()
{
  /*
  auto pqCore = pqApplicationCore::instance();
  if (pqCore)
  {
  }
  */
}
