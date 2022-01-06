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
#ifndef pqCrinklePropertyWidget_h
#define pqCrinklePropertyWidget_h

#include "smtk/extension/paraview/widgets/pqSlicePropertyWidgetBase.h"

/**
 * pqCrinklePropertyWidget is a custom property widget that accepts a
 * plane origin and normal, then renders a slice of a user-selected
 * subset of the input dataset's blocks that pass through the plane.
 */
class SMTKPQWIDGETSEXT_EXPORT pqCrinklePropertyWidget : public pqSlicePropertyWidgetBase
{
  Q_OBJECT
  typedef pqSlicePropertyWidgetBase Superclass;

public:
  /**\brief Create a widget for slicing an \a input dataset.
    *
    * The \a proxy references some object whose \a smgroup contains
    * an Origin and a Normal property defining the slice plane.
    * Each of these properties is expected to hold 3 floating-point values.
    */
  pqCrinklePropertyWidget(
    pqPipelineSource* input,
    vtkSMProxy* proxy,
    vtkSMPropertyGroup* smgroup,
    QWidget* parent = nullptr);
  ~pqCrinklePropertyWidget() override;

private:
  Q_DISABLE_COPY(pqCrinklePropertyWidget)
};

#endif
