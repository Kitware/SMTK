//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_widgets_pqSMTKSphereItemWidget_h
#define smtk_extension_paraview_widgets_pqSMTKSphereItemWidget_h

#include "smtk/extension/paraview/widgets/pqSMTKAttributeItemWidget.h"

/**\brief Display a 3-D sphere with draggable handles
  *       for editing a GroupItem with 1 vector and 1 scalar.
  *
  * For now, this code assumes that the Group has 2 entries and they
  * that its AttributeItemInfo entry specify a mapping to the
  * Center and Radius of the sphere.
  * In the future, other item types (such as 3 DoubleItem holding
  * 3 points used to bound the sphere) may be supported.
  */
class SMTKPQWIDGETSEXT_EXPORT pqSMTKSphereItemWidget : public pqSMTKAttributeItemWidget
{
  Q_OBJECT
public:
  pqSMTKSphereItemWidget(
    const smtk::extension::AttributeItemInfo& info, Qt::Orientation orient = Qt::Horizontal);
  virtual ~pqSMTKSphereItemWidget();

  static qtItem* createSphereItemWidget(const AttributeItemInfo& info);
  bool createProxyAndWidget(vtkSMProxy*& proxy, pqInteractivePropertyWidget*& widget) override;
  void updateItemFromWidget() override;

protected:
  /**\brief Starting with the widget's assigned item (which must
    *       be a GroupItem), fetch the proper children.
    *
    * If errors are encountered, this method returns false.
    */
  bool fetchCenterAndRadiusItems(
    smtk::attribute::DoubleItemPtr& centerItem, smtk::attribute::DoubleItemPtr& radiusItem);
};

#endif // smtk_extension_paraview_widgets_pqSMTKSphereItemWidget_h
