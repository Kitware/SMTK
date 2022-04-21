//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_widgets_pqSMTKLineItemWidget_h
#define smtk_extension_paraview_widgets_pqSMTKLineItemWidget_h

#include "smtk/extension/paraview/widgets/pqSMTKAttributeItemWidget.h"

/**\brief Display a 3-D line with draggable handles for editing a GroupItem.
  *
  * For now, this code assumes that the Group has 2 entries and
  * their qtAttributeItemInfo entries specify a mapping to the
  * endpoints of the line (named Point1 and Point2).
  * In the future, other item types (such as defining a line by a
  * a base point, direction, and optionally length) may be supported.
  *
  * Currently, there is no support to initialize the bounding box
  * coordinates used to size the line widget;
  * the item's values will be copied to the 3-D representation only if
  * they exist and there is no default or if they are non-default.
  * In the future, flags in the qtAttributeItemInfo may be used to
  * determine a default box based on model geometry loaded into ParaView,
  * as the underlying widget supports this.
  */
class SMTKPQWIDGETSEXT_EXPORT pqSMTKLineItemWidget : public pqSMTKAttributeItemWidget
{
  Q_OBJECT
public:
  pqSMTKLineItemWidget(
    const smtk::extension::qtAttributeItemInfo& info,
    Qt::Orientation orient = Qt::Horizontal);
  ~pqSMTKLineItemWidget() override;

  static qtItem* createLineItemWidget(const qtAttributeItemInfo& info);
  bool createProxyAndWidget(vtkSMProxy*& proxy, pqInteractivePropertyWidget*& widget) override;

protected Q_SLOTS:
  void updateItemFromWidgetInternal() override;

protected:
  /**\brief Starting with the widget's assigned item (which must
    *       be a GroupItem), fetch the proper children.
    *
    * If errors are encountered, this method returns false.
    */
  bool fetchEndpointItems(
    smtk::attribute::DoubleItemPtr& endpt1,
    smtk::attribute::DoubleItemPtr& endpt2);
};

#endif // smtk_extension_paraview_widgets_pqSMTKLineItemWidget_h
