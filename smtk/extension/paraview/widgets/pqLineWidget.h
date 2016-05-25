//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_pq_LineWidget_h
#define __smtk_pq_LineWidget_h

#include "smtk/extension/paraview/widgets/Exports.h"
#include "pq3DWidget.h"
#include <QColor>

class pqServer;

/// Provides a complete Qt UI for working with a 3D line widget
class SMTKPQWIDGETSEXT_EXPORT pqLineWidget : public pq3DWidget
{
  typedef pq3DWidget Superclass;
  
  Q_OBJECT
  
public:
  pqLineWidget(vtkSMProxy* o, vtkSMProxy* pxy, QWidget* p = 0, 
    const char* xmlname="LineWidgetRepresentation");
  ~pqLineWidget();

  /// Resets the bounds of the 3D widget to the reference proxy bounds.
  /// This typically calls PlaceWidget on the underlying 3D Widget 
  /// with reference proxy bounds.
  /// This should be explicitly called after the panel is created
  /// and the widget is initialized i.e. the reference proxy, controlled proxy
  /// and hints have been set.
  virtual void resetBounds()
    { this->Superclass::resetBounds(); }
  virtual void resetBounds(double bounds[6]);

  void setControlledProperties(vtkSMProperty* point1, vtkSMProperty* point2);
  void setLineColor(const QColor& color);

public slots:
  void onXAxis();
  void onYAxis();
  void onZAxis();

protected:
  virtual void setControlledProperty(const char* function,
    vtkSMProperty * controlled_property);

  /// Called on pick.
  virtual void pick(double, double, double);

private slots:
  void onWidgetVisibilityChanged(bool visible);

private:
  void createWidget(pqServer* server, const QString& xmlname);
  void getReferenceBoundingBox(double center[3], double size[3]);

  class pqImplementation;
  pqImplementation* const Implementation;
};

#endif
