//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_extension_paraview_widgets_pqSMTKInteractivePropertyWidget_h
#define smtk_extension_paraview_widgets_pqSMTKInteractivePropertyWidget_h

#include "smtk/extension/paraview/widgets/smtkPQWidgetsExtModule.h"

#include "pqInteractivePropertyWidget.h"

/**\brief Base class of ParaView property widgets that customizes visibility
  *
  * Add functionality to avoid hiding the 3D widget when the QT widget looses
  * focus or is hidden. Setting controlled by `pqSMTKAttributeItemWidget`.
  */
class SMTKPQWIDGETSEXT_EXPORT pqSMTKInteractivePropertyWidget : public pqInteractivePropertyWidget
{
  Q_OBJECT
  using Superclass = pqInteractivePropertyWidget;

public:
  pqSMTKInteractivePropertyWidget(
    const char* widget_smgroup,
    const char* widget_smname,
    vtkSMProxy* proxy,
    vtkSMPropertyGroup* smgroup,
    QWidget* parent = nullptr)
    : Superclass(widget_smgroup, widget_smname, proxy, smgroup, parent){};

  /// allow override of default behavior which hides the widget when not "selected"
  void deselect() override
  {
    if (this->hideWhenInactive())
    {
      Superclass::deselect();
    }
    // else do nothing so the widget is always selected.
  };

  /// allow override to not hide 3D widget when QT widget is hidden.
  void hideEvent(QHideEvent* e) override
  {
    Superclass::hideEvent(e);
    // if we don't want to hide, show if the user turned it on.
    if (!this->hideWhenInactive() && this->VisibleState)
    {
      this->setWidgetVisible(true);
    }
  };

  void setHideWhenInactive(bool val) { m_hideWhenInactive = val; };
  bool hideWhenInactive() const { return m_hideWhenInactive; };

protected:
  bool m_hideWhenInactive = true;

private:
  Q_DISABLE_COPY(pqSMTKInteractivePropertyWidget);
};

#endif //smtk_extension_paraview_widgets_pqSMTKInteractivePropertyWidget_h
