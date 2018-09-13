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

#include "smtk/extension/paraview/widgets/Exports.h"
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
  using AttributeItemInfo = smtk::extension::AttributeItemInfo;

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

  pqSMTKAttributeItemWidget(const AttributeItemInfo& info, Qt::Orientation orient = Qt::Horizontal);
  pqSMTKAttributeItemWidget(smtk::attribute::ItemPtr, QWidget* p,
    smtk::extension::qtBaseView* bview, Qt::Orientation orient = Qt::Horizontal);
  virtual ~pqSMTKAttributeItemWidget();

  /// Subclasses must override this method to create the ParaView widget of their choice.
  virtual bool createProxyAndWidget(vtkSMProxy*& source, pqInteractivePropertyWidget*& widget) = 0;

public slots:
  virtual void updateItemFromWidget() = 0;

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

protected slots:
  virtual void updateItemData();
  virtual void acceptWidgetValues();

protected:
  virtual void createWidget();
  virtual void clearChildWidgets();
  virtual void updateUI();
  virtual void createEditor();

  class Internal;
  Internal* m_p;
};

#endif // smtk_extension_paraview_widgets_pqSMTKAttributeItemWidget_h
