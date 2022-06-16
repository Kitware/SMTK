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
#include "smtk/extension/paraview/widgets/smtkPQWidgetsExtModule.h"

class QCheckBox;

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
class SMTKPQWIDGETSEXT_EXPORT pqSMTKBoxItemWidget : public pqSMTKAttributeItemWidget
{
  Q_OBJECT
public:
  pqSMTKBoxItemWidget(
    const smtk::extension::qtAttributeItemInfo& info,
    Qt::Orientation orient = Qt::Horizontal);
  ~pqSMTKBoxItemWidget() override;

  static qtItem* createBoxItemWidget(const qtAttributeItemInfo& info);
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
    /// 1 item with 6 values (xmin, xmax, ymin, ymax, zmin, zmax)
    AxisAlignedBounds,
    /// 2 items with 3 values each (xlo, ylo, zlo), (xhi, yhi, zhi)
    AxisAlignedMinMax,
    /// 2 items with 3 values each (xc, yc, zc), (dx, dy, dz)
    AxisAlignedCenterDeltas,
    /// 1 item with 6 values (min/max as above), 1 item with (roll, pitch, yaw)
    EulerAngleBounds,
    /// 3 items with 3 values each (xlo, ylo, zlo), (xhi, yhi, zhi), (roll, pitch, yaw)
    EulerAngleMinMax,
    /// 3 items with 3 values each (xc, yc, zc), (dx, dy, dz), (roll, pitch, yaw)
    EulerAngleCenterDeltas,
    /// No consistent set of items detected.
    Invalid
  };
  /**\brief Starting with the widget's assigned item (which may
    *       be a GroupItem or a DoubleItem), determine and return bound items.
    *
    * If errors are encountered, this method returns false.
    * If the name of a DoubleItem is provided, then the AxisAlignedBounds binding
    * is assumed and that item is returned as the sole entry of \items.
    * Otherwise, the named item must be a Group holding items called out as via
    * one of the remaining valid ItemBindings enumerants.
    *
    * Angles must be provided in degrees and are Tait-Bryan angles
    * as used by VTK (i.e., rotations about y then x then z axes).
    * These are similar to Euler angles and sometimes called as such.
    * See https://en.wikipedia.org/wiki/Euler_angles#Intrinsic_rotations
    * for more information. VTK uses the Y_1 X_2 Z_3 ordering.
    *
    * Finally, the \a control parameter may be set to null or populated with
    * a discrete-valued string item depending on whether the view configuration
    * for the item specifies the name of an appropriate item.
    * If non-null, the iteraction item will be tied to the "Show Box"
    * checkbox of paraview's Qt widget. Its discrete values must be
    * "active" and "inactive".
    */
  bool fetchBoxItems(
    ItemBindings& binding,
    std::vector<smtk::attribute::DoubleItemPtr>& items,
    smtk::attribute::StringItemPtr& control);

  void setControlState(const std::string& controlState, QCheckBox* controlWidget);
};

#endif // smtk_extension_paraview_widgets_pqSMTKBoxItemWidget_h
