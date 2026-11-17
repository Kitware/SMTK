//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_widgets_pqSMTKConePropertyWidget_h
#define smtk_extension_paraview_widgets_pqSMTKConePropertyWidget_h

#include "smtk/extension/paraview/widgets/pqSMTKInteractivePropertyWidget.h"
#include "smtk/extension/paraview/widgets/smtkPQWidgetsExtModule.h"

class SMTKPQWIDGETSEXT_EXPORT pqSMTKConePropertyWidget : public pqSMTKInteractivePropertyWidget
{
  Q_OBJECT
  using Superclass = pqSMTKInteractivePropertyWidget;

public:
  pqSMTKConePropertyWidget(
    vtkSMProxy* proxy,
    vtkSMPropertyGroup* smgroup,
    QWidget* parent = nullptr);
  ~pqSMTKConePropertyWidget() override;

public Q_SLOTS:
  void pick(double, double, double);
  void pickPoint1(double, double, double);
  void pickPoint2(double, double, double);
  /// Force the widget to accept a single, positive radius (when true).
  void setCylindrical(bool);
  /// The same as setCylindrical, but also hide the checkbox that
  /// allows user control of whether the cone is a cylinder (when true).
  void setForceCylindrical(bool);

protected Q_SLOTS:
  void updateInformationLabels();
  void placeWidget() override;

protected:
  class Internals;
  Internals* m_p;

private:
  Q_DISABLE_COPY(pqSMTKConePropertyWidget);
};

#endif // smtk_extension_paraview_widgets_pqSMTKConePropertyWidget_h
