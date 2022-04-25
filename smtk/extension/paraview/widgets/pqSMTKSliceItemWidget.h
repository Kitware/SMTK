//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_widgets_pqSMTKSliceItemWidget_h
#define smtk_extension_paraview_widgets_pqSMTKSliceItemWidget_h

#include "smtk/extension/paraview/widgets/pqSMTKAttributeItemWidget.h"

/**\brief Display a 3-D plane with draggable handles for editing a GroupItem.
  *
  * For now, this code assumes that the Group has 2 entries and they
  * that their qtAttributeItemInfo entries specify a mapping to the
  * Origin and Normal of the plane.
  * In the future, other item types (such as a DoubleItem holding
  * 4 values used as linear polynomial coefficients) may be supported.
  *
  * Currently, there is no support to initialize the bounding box
  * coordinates used to size/place the plane widget;
  * the item's values will be copied to the 3-D representation only if
  * they exist and there is no default or if they are non-default.
  * In the future, flags in the qtAttributeItemInfo may be used to
  * determine a default box based on model geometry loaded into ParaView,
  * as the underlying widget supports this.
  */
class SMTKPQWIDGETSEXT_EXPORT pqSMTKSliceItemWidget : public pqSMTKAttributeItemWidget
{
  Q_OBJECT
public:
  pqSMTKSliceItemWidget(
    const smtk::extension::qtAttributeItemInfo& info,
    Qt::Orientation orient = Qt::Horizontal);
  ~pqSMTKSliceItemWidget() override;

  static qtItem* createSliceItemWidget(const qtAttributeItemInfo& info);
  bool createProxyAndWidget(vtkSMProxy*& proxy, pqInteractivePropertyWidget*& widget) override;

public Q_SLOTS:
  void sliceInputsChanged();

protected Q_SLOTS:
  /// Retrieve property values from ParaView proxy and store them in the attribute's Item.
  void updateItemFromWidgetInternal() override;
  /// Retrieve values from the attribute system and update the ParaView proxy.
  void updateWidgetFromItemInternal() override;

protected:
  /**\brief Starting with the widget's assigned item (which must
    *       be a GroupItem), fetch the proper children.
    *
    * If errors are encountered, this method returns false.
    */
  bool fetchOriginNormalAndInputsItems(
    smtk::attribute::DoubleItemPtr& originItem,
    smtk::attribute::DoubleItemPtr& normalItem,
    smtk::attribute::ReferenceItemPtr& inputsItem);
};

#endif // smtk_extension_paraview_widgets_pqSMTKSliceItemWidget_h
