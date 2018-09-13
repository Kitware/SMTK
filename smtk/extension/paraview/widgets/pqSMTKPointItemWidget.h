//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_widgets_pqSMTKPointItemWidget_h
#define smtk_extension_paraview_widgets_pqSMTKPointItemWidget_h

#include "smtk/extension/paraview/widgets/pqSMTKAttributeItemWidget.h"

/**\brief Display a 3-D point with draggable handles for editing a DoubleItem.
  *
  * For now, this code assumes that the item is a DoubleItem with 3 values.
  * In the future, other item types may be supported.
  *
  * Currently, there is no support to initialize the bounding box
  * coordinates used to place the point widget;
  * the item's values will be copied to the 3-D representation only if
  * they exist and there is no default or if they are non-default.
  * In the future, flags in the AttributeItemInfo may be used to
  * determine a default box based on model geometry loaded into ParaView,
  * as the underlying widget supports this.
  */
class SMTKPQWIDGETSEXT_EXPORT pqSMTKPointItemWidget : public pqSMTKAttributeItemWidget
{
  Q_OBJECT
public:
  pqSMTKPointItemWidget(
    const smtk::extension::AttributeItemInfo& info, Qt::Orientation orient = Qt::Horizontal);
  virtual ~pqSMTKPointItemWidget();

  static qtItem* createPointItemWidget(const AttributeItemInfo& info);
  bool createProxyAndWidget(vtkSMProxy*& proxy, pqInteractivePropertyWidget*& widget) override;
  void updateItemFromWidget() override;

protected:
  bool fetchPointItem(smtk::attribute::DoubleItemPtr& pointItem);
};

#endif // smtk_extension_paraview_widgets_pqSMTKPointItemWidget_h
