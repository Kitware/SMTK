//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_widgets_pqSMTKDiskItemWidget_h
#define smtk_extension_paraview_widgets_pqSMTKDiskItemWidget_h

#include "smtk/extension/paraview/widgets/pqSMTKAttributeItemWidget.h"

/**\brief Display an interactive disk widget.
  *
  * The widget currently accepts a center point, normal, and radius
  * that defines 2-d disk embedded in 3-d. These values should be
  * child double-items of an SMTK group. Your view-configuration XML
  * should mention this item explicitly and specify the path to the
  * group as well as the names of individual child items like so:
  *
  * ```xml
  *      <ItemViews>
  *        <View Type="Disk" Item="NameOfGroup"
  *          Center="NameOfCenterItemInsideGroup"
  *          Normal="NameOfNormalItemInsideGroup"
  *          Radius="NameOfRadiusItemInsideGroup"
  *          ShowControls="true"/>
  *      </ItemViews>
  *    </Att>
  * ```
  *
  * In the future, other item types (such as a GroupItem holding
  * children specifying 3 points on the circumference of a disk;
  * or a center point and point on the circumferece; or a pair of
  * diametrically opposed points and a normal) may be supported.
  *
  * Currently, there is no support to initialize the placement to
  * a given set of bounds; if you use this widget as part of the
  * user interface to an operation, implement a configure() method
  * on the operation to contextually place the widget based on
  * associations.
  */
class SMTKPQWIDGETSEXT_EXPORT pqSMTKDiskItemWidget : public pqSMTKAttributeItemWidget
{
  Q_OBJECT
public:
  pqSMTKDiskItemWidget(
    const smtk::extension::qtAttributeItemInfo& info,
    Qt::Orientation orient = Qt::Horizontal);
  ~pqSMTKDiskItemWidget() override;

  /// Create an instance of the widget that allows users to define a disk.
  static qtItem* createDiskItemWidget(const qtAttributeItemInfo& info);

  bool createProxyAndWidget(vtkSMProxy*& proxy, pqInteractivePropertyWidget*& widget) override;

protected Q_SLOTS:
  /// Retrieve property values from ParaView proxy and store them in the attribute's Item.
  bool updateItemFromWidgetInternal() override;
  /// Retrieve property values from the attribute's Item and update the ParaView proxy.
  bool updateWidgetFromItemInternal() override;

protected:
  /// Describe how an attribute's items specify a disk or cylinder.
  enum class ItemBindings
  {
    /// 2 items with 3 values, 1 items with 1 value (cx, cy, cz, nx, ny, nz, rr).
    DiskPointsRadii,
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
  bool fetchDiskItems(ItemBindings& binding, std::vector<smtk::attribute::DoubleItemPtr>& items);
};

#endif // smtk_extension_paraview_widgets_pqSMTKDiskItemWidget_h
