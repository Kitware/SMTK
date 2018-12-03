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

protected:
  enum class ItemBindings
  {
    AxisAlignedBounds,       //!< 1 item with 6 values (xmin, xmax, ymin, ymax, zmin, zmax)
    AxisAlignedMinMax,       //!< 2 items with 3 values each (xlo, ylo, zlo), (xhi, yhi, zhi)
    AxisAlignedCenterDeltas, //!< 2 items with 3 values each (xc, yc, zc), (dx, dy, dz)
    EulerAngleMinMax, //!< 3 items with 3 values each (xlo, ylo, zlo), (xhi, yhi, zhi), (roll, pitch, yaw)
    EulerAngleCenterDeltas //!< 3 items with 3 values each (xc, yc, zc), (dx, dy, dz), (roll, pitch, yaw)
  };
  /**\brief Starting with the widget's assigned item (which may
    *       be a GroupItem or a DoubleItem), determine and return bound items.
    *
    * If errors are encountered, this method returns false.
    * If the name of a DoubleItem is provided, then the AxisAlignedBounds binding
    * is assumed and that item is returned as the sole entry of \items.
    * Otherwise, the named item must be a Group holding items called out as one
    * of the following:
    * + AxisAlignedMinMax: "Min", "Max" with numberOfValues == 3
    * + AxisAlignedCenterDeltas: "Center", "Deltas", with numberOfValues == 3
    * + EulerAngleMinMax: "Angles", "Min", "Max" with numberOfValues == 3
    * + EulerAngleCenterDeltas: "Angles, "Center", "Deltas" with numberOfValues == 3
    *
    * Euler angles must be provided in degrees and are roll, pitch, and yaw
    * (i.e., rotation about the x, y, and z axes, respectively).
    */
  bool fetchBoxItems(ItemBindings& binding, std::vector<smtk::attribute::DoubleItemPtr>& items);
};

#endif // smtk_extension_paraview_widgets_pqSMTKBoxItemWidget_h
