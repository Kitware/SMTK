//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_widgets_pqSMTKBoxItemWidget_h
#define smtk_extension_paraview_widgets_pqSMTKBoxItemWidget_h

#include "smtk/extension/paraview/widgets/pqSMTKAttributeItemWidget.h"

/**\brief Display a 3-D box with draggable handles for editing a DoubleItem.
  *
  * For now, this code assumes that the DoubleItem has 6 entries and they
  * are ordered (xmin, xmax, ymin, ymax, zmin, zmax).
  * It also assumes the box must be axis-aligned (since there is no
  * way to store rotations in addition to the 6 double values without
  * getting fancy).
  * In the future, other item types (such as a GroupItem holding
  * children specifying corner points and rotations) may be supported.
  *
  * Currently, there is no support to initialize the box coordinates;
  * the item's values will be copied to the 3-D representation only if
  * they exist and there is no default or if they are non-default.
  * In the future, flags in the AttributeItemInfo may be used to
  * determine a default box based on model geometry loaded into ParaView,
  * as the underlying widget supports this.
  */
class SMTKPQWIDGETSEXT_EXPORT pqSMTKBoxItemWidget : public pqSMTKAttributeItemWidget
{
  Q_OBJECT
public:
  pqSMTKBoxItemWidget(
    const smtk::extension::AttributeItemInfo& info, Qt::Orientation orient = Qt::Horizontal);
  virtual ~pqSMTKBoxItemWidget();

  static qtItem* createBoxItemWidget(const AttributeItemInfo& info);
  bool createProxyAndWidget(vtkSMProxy*& proxy, pqInteractivePropertyWidget*& widget) override;
  void updateItemFromWidget() override;
};

#endif // smtk_extension_paraview_widgets_pqSMTKBoxItemWidget_h
