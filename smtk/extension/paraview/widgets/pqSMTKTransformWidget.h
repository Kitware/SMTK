//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_widgets_pqSMTKTransformWidget_h
#define smtk_extension_paraview_widgets_pqSMTKTransformWidget_h

#include "smtk/extension/paraview/widgets/pqSMTKAttributeItemWidget.h"
#include "smtk/extension/paraview/widgets/smtkPQWidgetsExtModule.h"

class QCheckBox;

/**\brief Transform an SMTK component by manipulating its bounding box.
  */
class SMTKPQWIDGETSEXT_EXPORT pqSMTKTransformWidget : public pqSMTKAttributeItemWidget
{
  Q_OBJECT
public:
  pqSMTKTransformWidget(
    const smtk::extension::qtAttributeItemInfo& info,
    Qt::Orientation orient = Qt::Horizontal);
  ~pqSMTKTransformWidget() override;

  static qtItem* createTransformWidget(const qtAttributeItemInfo& info);
  bool createProxyAndWidget(vtkSMProxy*& proxy, pqInteractivePropertyWidget*& widget) override;

protected Q_SLOTS:
  void resetWidget();
  /// Retrieve property values from ParaView proxy and store them in the attribute's Item.
  bool updateItemFromWidgetInternal() override;
  /// Retrieve property values from the attribute's Item and update the ParaView proxy.
  bool updateWidgetFromItemInternal() override;

protected:
  bool fetchTransformItems(
    std::vector<smtk::attribute::DoubleItemPtr>& items,
    smtk::attribute::StringItemPtr& control);

  void setControlState(const std::string& controlState, QCheckBox* controlWidget);

private:
  struct Internal;
  std::unique_ptr<Internal> m_internal;
};

#endif // smtk_extension_paraview_widgets_pqSMTKTransformWidget_h
