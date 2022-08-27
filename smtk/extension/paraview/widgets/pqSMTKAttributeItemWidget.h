//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_widgets_pqSMTKAttributeItemWidget_h
#define smtk_extension_paraview_widgets_pqSMTKAttributeItemWidget_h

#include "smtk/extension/paraview/widgets/smtkPQWidgetsExtModule.h"
#include "smtk/extension/qt/qtItem.h"

class vtkSMProxy;
class pqInteractivePropertyWidget;

/**\brief A subclass of qtItem common to all ParaView property widgets.
  *
  * SMTK's ParaView extensions provide custom qtItem subclasses that allow
  * users to edit item values with 2-D/3-D widgets in the active view.
  * This abstract class handles as much of the commonalities between the
  * ParaView-widget-specific subclasses as possible.
  *
  * In particular, it creates Qt widgets to hold the ParaView GUI
  * components while also matching the style of other qtItem
  * subclasses; it manages a checkbox enabling/disabling the widget
  * if the item's IsOptional flag is true, it creates a label showing
  * the item's title, etc.
  */
class SMTKPQWIDGETSEXT_EXPORT pqSMTKAttributeItemWidget : public smtk::extension::qtItem
{
  Q_OBJECT
public:
  using qtItem = smtk::extension::qtItem;
  using qtAttributeItemInfo = smtk::extension::qtAttributeItemInfo;

  void markForDeletion() override;

  /**\brief Specify when the widget may override values in the item.
    *
    */
  enum class OverrideWhen
  {
    Unset, //!< When the item is unset and FallbackStrategy is Force.
    Never  //!< Do not ever let the widget dictate values in the item without user input.
  };

  /**\brief Specify how to size/place a widget.
    *
    */
  enum class FallbackStrategy
  {
    Hide, //!< Hide the widget rather than show an uninitialized widget.
    Force //!< Force the widget's size/placement to either the geometry source or the item's default.
  };

  /**\brief Specify how to size/place a widget.
    *
    */
  enum class GeometrySource
  {
    Item,         //!< Use the item's current value(s).
    Associations, //!< Use the attribute's current association(s).
    Links,        //!< Use the attribute-resource's linked resources.
    Scene,        //!< Use all geometric resources in the current scene.
    BestGuess,    //!< Use all of the above, falling back as needed.
    None          //!< Do not use model/mesh resources to initialize the widget.
  };

  // Convert enums to/from strings
  static OverrideWhen OverrideWhenConvert(const std::string& str);
  static std::string OverrideWhenConvert(OverrideWhen val);

  static FallbackStrategy FallbackStrategyConvert(const std::string& str);
  static std::string FallbackStrategyConvert(FallbackStrategy val);

  static GeometrySource GeometrySourceConvert(const std::string& str);
  static std::string GeometrySourceConvert(GeometrySource val);

  /// Set visibility behavior of 3D widgets when Qt widget is inactive.
  static void setHideWidgetWhenInactive(bool val);
  static bool hideWidgetWhenInactive();

  pqSMTKAttributeItemWidget(
    const qtAttributeItemInfo& info,
    Qt::Orientation orient = Qt::Horizontal);
  pqSMTKAttributeItemWidget(
    smtk::attribute::ItemPtr,
    QWidget* p,
    smtk::extension::qtBaseView* bview,
    Qt::Orientation orient = Qt::Horizontal);
  ~pqSMTKAttributeItemWidget() override;

  /// Subclasses must override this method to create the ParaView widget of their choice.
  virtual bool createProxyAndWidget(vtkSMProxy*& source, pqInteractivePropertyWidget*& widget) = 0;
  pqInteractivePropertyWidget* propertyWidget();

public Q_SLOTS:
  virtual void updateItemFromWidget();
  virtual void updateWidgetFromItem();

  /**\brief Change whether the item is enabled (and thus the widget active).
    *
    * Note that subclasses *must* override this method and, at a minimum,
    * set the underlying smtk::attribute::Item's optional state.
    * Subclasses must do this because ParaView widgets can represent items
    * of different types.
    *
    * However, subclasses may wish to call this method from within their
    * override as it will enable/disable the paraview property widget.
    */
  void setOutputOptional(int optionEnabled);

  /**\brief Cause the active view (in which the widget appears) to re-render.
    */
  void renderViewEventually() const;

protected Q_SLOTS:
  /// Create Qt widgets as required (may be called multiple times if Item is reconfigured).
  void updateItemData() override;
  virtual void ignoreWidgetValues();
  virtual void acceptWidgetValues();
  virtual void updateItemFromWidgetInternal() = 0;
  virtual void updateWidgetFromItemInternal() {}

protected:
  /// An event filter that watches for the associated Qt widget to be hidden
  /// or shown. When this happens, update3DWidgetVisibility() is invoked.
  bool eventFilter(QObject* obj, QEvent* event) override;
  /// Hide 3-d widgets when the Qt widget is hidden, show the widget when
  /// the Qt widget reappears.
  ///
  /// Subclasses may override update3DWidgetVisibility() if they do not
  /// want to inherit the default behavior.
  virtual void update3DWidgetVisibility(bool visible);

  /// Initialize Qt widgets used to represent our smtk::attribute::Item.
  void createWidget() override;
  /**\brief Remove existing widgets in order to prepare for reconfiguration.
    *
    * If conditional children exist, this may get called after createWidget().
    */
  virtual void clearChildWidgets();
  virtual void updateUI();
  virtual void createEditor();

  /// Subclasses may call this to validate that a string item is
  /// appropriate for controlling the interaction state of the
  /// widget (i.e., the possibly tri-state checkbox that ParaView's
  /// Qt widgets provide to control render-view widget visibility).
  bool validateControlItem(const smtk::attribute::StringItemPtr& item);

  class Internal;
  Internal* m_p;
};

#endif // smtk_extension_paraview_widgets_pqSMTKAttributeItemWidget_h
