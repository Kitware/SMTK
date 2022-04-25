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
  * This class accepts an item that a DoubleItem with 3 values, or
  * with additional configuration parameters, a GroupItem containing
  * such a DoubleItem along with a discrete StringItem used to control
  * the display and interaction state of the widget.
  * Additionally, the controlling StringItem may be specified via a
  * path to that item rather than its membership in the same group as
  * the DoubleItem.
  *
  * To configure an item to use this widget, use Type="Point" in the
  * item's view configuration XML.
  * If the item you are configuring is a Double item, no further
  * configuration is required.
  * If the item is a Group containing a Double item that holds point
  * coordinates, you must also provide an attribute named _Coords_
  * that specifies the name of the item inside the group that holds
  * point coordinates.
  * You may always optionally provide
  * + a _ShowControls_ attribute set to "true" or "false" indicating
  *   whether controls for the point visibility/interaction are
  *   added as part of the point widget or not.
  * + a _Control_ attribute naming a discrete String item if you
  *   would like to save the visibility and interaction state of
  *   the widget in the attribute system.
  *
  * The controls for interaction state allow users to specify whether
  * the point widget is displayed in render windows as well as whether
  * keyboard shortcuts are registered for it.
  * The UI controls may be hidden or displayed independent of whether
  * the controls are bound to an SMTK item.
  *
  * Currently, there is no support to initialize the bounding box
  * coordinates used to place the point widget;
  * the item's values will be copied to the 3-D representation only if
  * they exist and there is no default or if they are non-default.
  * In the future, flags in the qtAttributeItemInfo may be used to
  * determine a default box based on model geometry loaded into ParaView,
  * as the underlying widget supports this.
  */
class SMTKPQWIDGETSEXT_EXPORT pqSMTKPointItemWidget : public pqSMTKAttributeItemWidget
{
  Q_OBJECT
public:
  pqSMTKPointItemWidget(
    const smtk::extension::qtAttributeItemInfo& info,
    Qt::Orientation orient = Qt::Horizontal);
  ~pqSMTKPointItemWidget() override;

  static qtItem* createPointItemWidget(const qtAttributeItemInfo& info);
  bool createProxyAndWidget(vtkSMProxy*& proxy, pqInteractivePropertyWidget*& widget) override;

protected Q_SLOTS:
  void updateItemFromWidgetInternal() override;
  void updateWidgetFromItemInternal() override;

protected:
  /// SMTK attribute items to which the widget's values are bound.
  enum class ItemBindings
  {
    PointCoords,           //!< 1 double-item with 3 required values
    PointCoordsAndControl, //!< 1 double-item with 3 required values and 1 discrete string-item
    Invalid                //!< No unique, preferred binding could be discovered.
  };

  /**\brief Discover the item binding from the configuration
    *       and return the corresponding items.
    *
    * If no valid binding exists, false will be returned and \a binding will be
    * set to Invalid.
    *
    * In order to be a control item, a string item must be discrete
    * and have the following enumerants: "active" (i.e., the point is
    * displayed and accepting keyboard shortcuts), "visible" (i.e.,
    * the point is displayed but no shortcuts are active), and
    * "inactive" (i.e., the point is neither displayed nor accepting
    * shortcuts to set its value).
    */
  bool fetchPointItems(
    ItemBindings& binding,
    smtk::attribute::DoubleItemPtr& coordItem,
    smtk::attribute::StringItemPtr& controlItem);
};

#endif // smtk_extension_paraview_widgets_pqSMTKPointItemWidget_h
