//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_widgets_pqPointPickingVisibilityHelper_h
#define smtk_extension_paraview_widgets_pqPointPickingVisibilityHelper_h

#include "pqInteractivePropertyWidget.h"
#include "smtk/extension/paraview/widgets/smtkPQWidgetsExtModule.h"

/// A utility to adapt PV point-picking helpers across an API change.
template <typename PickHelper>
struct pqPointPickingVisibilityHelper
{
  // pqPointPickingHelper::setShortcutEnabled is no longer available.
  template <typename Test>
  static void test(pqInteractivePropertyWidget&, Test&, ...)
  {
    // Do nothing
  }

  // Older ParaView requires pqPointPickingHelper::setShortcutEnabled be connected.
  template <typename Test>
  static void test(pqInteractivePropertyWidget& w, Test& obj, decltype(&Test::setShortcutEnabled))
  {
    QObject::connect(
      &w, &pqInteractivePropertyWidget::widgetVisibilityUpdated, &obj, &Test::setShortcutEnabled);
  }

  pqPointPickingVisibilityHelper(pqInteractivePropertyWidget& w, PickHelper& obj)
  {
    pqPointPickingVisibilityHelper::test<PickHelper>(w, obj);
  }
};

#endif // smtk_extension_paraview_widgets_pqPointPickingVisibilityHelper_h
