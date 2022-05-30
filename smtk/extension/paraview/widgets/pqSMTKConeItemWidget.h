//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_widgets_pqSMTKConeItemWidget_h
#define smtk_extension_paraview_widgets_pqSMTKConeItemWidget_h

#include "smtk/extension/paraview/widgets/pqSMTKAttributeItemWidget.h"

/**\brief Display an interactive 3-D cone (or cylinder) frustum widget.
  *
  * The widget currently accepts 2 endpoints that define an
  * axis for the cone and, at the same time, 2 parallel planes
  * that cut the cone.
  * Each endpoint may have an associated radius, one of which may
  * be zero.
  *
  * If setForceCylindrical(true) is called, then the cone is forced to be a
  * cylinder with a single radius value accepted.
  *
  * Note that the widget does not allow an interior apex (i.e., neither
  * of the the radii may be negative).

  * In the future, other item types (such as a GroupItem holding
  * children specifying a center, Euler angles, and a length)
  * may be supported.
  *
  * Currently, there is no support to initialize the placement to
  * a given set of bounds; if you use this widget as part of the
  * user interface to an operation, implement a configure() method
  * on the operation to contextually place the widget based on
  * associations.
  */
class SMTKPQWIDGETSEXT_EXPORT pqSMTKConeItemWidget : public pqSMTKAttributeItemWidget
{
  Q_OBJECT
public:
  pqSMTKConeItemWidget(
    const smtk::extension::qtAttributeItemInfo& info,
    Qt::Orientation orient = Qt::Horizontal);
  ~pqSMTKConeItemWidget() override;

  /// Create an instance of the widget that allows users to define a cone.
  static qtItem* createConeItemWidget(const qtAttributeItemInfo& info);
  /// Create an instance of the widget that allows users to define a cylinder.
  static qtItem* createCylinderItemWidget(const qtAttributeItemInfo& info);

  bool createProxyAndWidget(vtkSMProxy*& proxy, pqInteractivePropertyWidget*& widget) override;

protected Q_SLOTS:
  /// Retrieve property values from ParaView proxy and store them in the attribute's Item.
  bool updateItemFromWidgetInternal() override;
  /// Retrieve property values from the attribute's Item and update the ParaView proxy.
  bool updateWidgetFromItemInternal() override;

public Q_SLOTS:
  /**\brief Change the user interface so that only cylinders are accepted (when passed true).
    *
    * This method returns true when the user interface changes state
    * as a result of the call and false otherwise.
    */
  bool setForceCylindrical(bool);

protected:
  /// Describe how an attribute's items specify a cone or cylinder.
  enum class ItemBindings
  {
    /// 2 items with 3 values, 2 items with 1 value (x0, y0, z0, x1, y1, z1, r0, r1).
    ConePointsRadii,
    /// 2 items with 3 values, 1 item with 1 value (x0, y0, z0, x1, y1, z1, radius).
    CylinderPointsRadius,
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
  bool fetchConeItems(ItemBindings& binding, std::vector<smtk::attribute::DoubleItemPtr>& items);

  bool m_forceCylinder{ false };
};

#endif // smtk_extension_paraview_widgets_pqSMTKConeItemWidget_h
