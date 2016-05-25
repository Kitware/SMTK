//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_pq_3DWidgetInterface_h
#define __smtk_pq_3DWidgetInterface_h

#include <QtPlugin>
#include "smtk/extension/paraview/widgets/Exports.h"

class pq3DWidget;
class vtkSMProxy;

/// Interface for plugins that provide pq3DWidget subclasses.
class SMTKPQWIDGETSEXT_EXPORT pq3DWidgetInterface
{
public:
  virtual ~pq3DWidgetInterface();

  /// Creates the 3D widget of the requested type is possible otherwise simply
  /// returns NULL.
  /// \c referenceProxy -- source proxy providing initialization data bounds
  ///                      etc.
  /// \c controlledProxy -- proxy whose properties are controlled by the 3D
  ///                       widget.
  virtual pq3DWidget* newWidget(const QString& name,
    vtkSMProxy* referenceProxy,
    vtkSMProxy* controlledProxy)=0;
};

Q_DECLARE_INTERFACE(pq3DWidgetInterface, "com.kitware/paraview/3dwidget")

#endif


