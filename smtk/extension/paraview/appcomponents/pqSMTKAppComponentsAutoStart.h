//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_pqSMTKAppComponentsAutoStart_h
#define smtk_extension_paraview_appcomponents_pqSMTKAppComponentsAutoStart_h

#include "smtk/extension/paraview/appcomponents/PluginExports.h"

#include <QObject>

class vtkSMProxy;
class pqPluginSMTKViewBehavior;

class SMTKPQCOMPONENTSPLUGIN_EXPORT pqSMTKAppComponentsAutoStart : public QObject
{
  Q_OBJECT
  using Superclass = QObject;

  static vtkSMProxy* resourceManager();

public:
  pqSMTKAppComponentsAutoStart(QObject* parent = nullptr);
  ~pqSMTKAppComponentsAutoStart() override;

  void startup();
  void shutdown();

protected:
  pqPluginSMTKViewBehavior* m_viewTracker;

private:
  Q_DISABLE_COPY(pqSMTKAppComponentsAutoStart);
};

#endif
