//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_widgets_pqSMTKInfiniteCylinderItemWidget_h
#define smtk_extension_paraview_widgets_pqSMTKInfiniteCylinderItemWidget_h

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
  * In the future, flags in the qtAttributeItemInfo may be used to
  * determine a default box based on model geometry loaded into ParaView,
  * as the underlying widget supports this.
  */
class SMTKPQWIDGETSEXT_EXPORT pqSMTKInfiniteCylinderItemWidget : public pqSMTKAttributeItemWidget
{
  Q_OBJECT
public:
  pqSMTKInfiniteCylinderItemWidget(
    const smtk::extension::qtAttributeItemInfo& info,
    Qt::Orientation orient = Qt::Horizontal);
  ~pqSMTKInfiniteCylinderItemWidget() override;

  static qtItem* createCylinderItemWidget(const qtAttributeItemInfo& info);
  bool createProxyAndWidget(vtkSMProxy*& proxy, pqInteractivePropertyWidget*& widget) override;

protected Q_SLOTS:
  /// Retrieve property values from ParaView proxy and store them in the attribute's Item.
  bool updateItemFromWidgetInternal() override;
  /// Retrieve property values from the attribute's Item and update the ParaView proxy.
  bool updateWidgetFromItemInternal() override;

protected:
  /// Describe how an attribute's items specify a bounding box.
  enum class ItemBindings
  {
    /// 2 items with 3 values, 1 item with 1 value (xc, yc, zc, xhl, yhl, zhl, radius)
    CenterHalfAxisRadius,
    /// No consistent set of items detected.
    Invalid
  };
  /**\brief Starting with the widget's assigned item (which must currently
    *       be a GroupItem), determine and return bound items.
    *
    * The named item must be a Group holding items as called out by
    * one of the valid ItemBindings enumerants.
    * The items inside the Group must currently be Double items.
    *
    * If errors (such as a lack of matching item names or an
    * unexpected number of values per item) are encountered,
    * this method returns false.
    */
  bool fetchCylinderItems(
    ItemBindings& binding,
    std::vector<smtk::attribute::DoubleItemPtr>& items);
};

#endif // smtk_extension_paraview_widgets_pqSMTKInfiniteCylinderItemWidget_h
