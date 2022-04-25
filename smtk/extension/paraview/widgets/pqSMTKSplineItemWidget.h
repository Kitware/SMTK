//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_widgets_pqSMTKSplineItemWidget_h
#define smtk_extension_paraview_widgets_pqSMTKSplineItemWidget_h

#include "smtk/extension/paraview/widgets/pqSMTKAttributeItemWidget.h"

class vtkEventQtSlotConnect;

/**\brief Display an editable planar spline in 3-D with draggable handles
  *       for editing a GroupItem containing a number of DoubleItem points.
  */
class SMTKPQWIDGETSEXT_EXPORT pqSMTKSplineItemWidget : public pqSMTKAttributeItemWidget
{
  Q_OBJECT
public:
  pqSMTKSplineItemWidget(
    const smtk::extension::qtAttributeItemInfo& info,
    Qt::Orientation orient = Qt::Horizontal);
  ~pqSMTKSplineItemWidget() override;

  static qtItem* createSplineItemWidget(const qtAttributeItemInfo& info);
  bool createProxyAndWidget(vtkSMProxy*& proxy, pqInteractivePropertyWidget*& widget) override;

protected Q_SLOTS:
  void updateItemFromWidgetInternal() override;

protected:
  /**\brief Starting with the widget's assigned item (which must
    *       be a GroupItem), fetch the proper children.
    *
    * If errors are encountered, this method returns false.
    */
  bool fetchPointsAndClosedItems(
    smtk::attribute::DoubleItemPtr& pointsItem,
    smtk::attribute::VoidItemPtr& closedItem);

  vtkEventQtSlotConnect* m_handleConnection;
};

#endif // smtk_extension_paraview_widgets_pqSMTKSplineItemWidget_h
