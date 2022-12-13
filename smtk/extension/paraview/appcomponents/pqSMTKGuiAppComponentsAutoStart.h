//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_pqSMTKGuiAppComponentsAutoStart_h
#define smtk_extension_paraview_appcomponents_pqSMTKGuiAppComponentsAutoStart_h

#include "smtk/extension/paraview/appcomponents/smtkPQComponentsExtModule.h"

#include "smtk/extension/paraview/appcomponents/pqQtKeywordWrapping.h"

#include <QObject>

class pqSMTKWrapper;
class pqServer;
class vtkSMProxy;

class SMTKPQCOMPONENTSEXT_EXPORT pqSMTKGuiAppComponentsAutoStart : public QObject
{
  Q_OBJECT
public:
  pqSMTKGuiAppComponentsAutoStart(QObject* parent = nullptr);
  ~pqSMTKGuiAppComponentsAutoStart() override = default;

  void startup();
  void shutdown();

private:
  Q_DISABLE_COPY(pqSMTKGuiAppComponentsAutoStart);
};

#endif
