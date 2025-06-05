//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_widgets_pqDiskPropertyWidget_h
#define smtk_extension_paraview_widgets_pqDiskPropertyWidget_h

#include "smtk/extension/paraview/widgets/pqSMTKInteractivePropertyWidget.h"
#include "smtk/extension/paraview/widgets/smtkPQWidgetsExtModule.h"

/**\brief Present a widget for placing a planar disc in a ParaView render-view.
  *
  * This widget allows users to choose the center point, normal, and radius of a
  * single disc in either the active 3-d view or manually inside a property panel.
  */
class SMTKPQWIDGETSEXT_EXPORT pqDiskPropertyWidget : public pqSMTKInteractivePropertyWidget
{
  Q_OBJECT
  using Superclass = pqSMTKInteractivePropertyWidget;

public:
  pqDiskPropertyWidget(vtkSMProxy* proxy, vtkSMPropertyGroup* smgroup, QWidget* parent = nullptr);
  ~pqDiskPropertyWidget() override;

public Q_SLOTS:
  void pick(double, double, double);

protected Q_SLOTS:
  void updateInformationLabels();
  void placeWidget() override;

protected:
  class Internals;
  Internals* m_p;

private:
  Q_DISABLE_COPY(pqDiskPropertyWidget);
};

#endif // smtk_extension_paraview_widgets_pqDiskPropertyWidget_h
