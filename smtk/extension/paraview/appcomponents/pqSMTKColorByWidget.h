//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_pqSMTKColorByWidget_h
#define smtk_extension_paraview_appcomponents_pqSMTKColorByWidget_h

#include "smtk/extension/paraview/appcomponents/smtkPQComponentsExtModule.h"

#include "pqPropertyWidget.h"

#include "smtk/extension/paraview/appcomponents/pqQtKeywordWrapping.h"

#include <QWidget>

class pqDataRepresentation;
class vtkSMProxy;

/**\brief A widget that allows users to choose an SMTK representation's color-by mode.
  *
  * This widget works with a vtkSMSMTKResourceRepresentationProxy, setting its
  * "ColorBy" property.
  *
  * In the future, when we support coloring by attribute or group, it will
  * provide an additional drop-down so users can select the attribute
  * definition or parent group as well.
  */
class SMTKPQCOMPONENTSEXT_EXPORT pqSMTKColorByWidget : public QWidget
{
  Q_OBJECT;
  Q_PROPERTY(QString colorByText READ colorByText WRITE setColorByText NOTIFY colorByTextChanged);
  typedef QWidget Superclass;

public:
  pqSMTKColorByWidget(QWidget* parent = nullptr);
  ~pqSMTKColorByWidget() override;

  /**
  * Returns the selected representation as a string.
  */
  QString colorByText() const;

public Q_SLOTS:
  /**
  * set the representation proxy or pqDataRepresentation instance.
  */
  void setRepresentation(pqDataRepresentation* display);
  void setRepresentation(vtkSMProxy* proxy);

  /**
  * set representation type.
  */
  void setColorByText(const QString&);

private Q_SLOTS:
  /**
  * Slot called when the combo-box is changed. If this change was due to
  * a UI interaction, we need to prompt the user if he really intended to make
  * that change (BUG #0015117).
  */
  void comboBoxChanged(const QString&);

Q_SIGNALS:
  void colorByTextChanged(const QString&);

  /**\brief Called when the colorByTextChanged argument moves to or from "Field".
    *
    * This is used by the pqSMTKColorByToolBar to enable/disable the "Active
    * Variable Controls" toolbar so it is only visible when it will have an effect.
    */
  void colorByFieldActive(bool active);

private:
  Q_DISABLE_COPY(pqSMTKColorByWidget)

  class pqInternals;
  pqInternals* Internal;

  class PropertyLinksConnection;
};

/**
* A property widget for selecting the display representation.
*/
class SMTKPQCOMPONENTSEXT_EXPORT pqSMTKColorByPropertyWidget : public pqPropertyWidget
{
  Q_OBJECT

public:
  pqSMTKColorByPropertyWidget(vtkSMProxy* proxy, QWidget* parent = nullptr);
  ~pqSMTKColorByPropertyWidget() override;

private:
  pqSMTKColorByWidget* Widget;
};

#endif
