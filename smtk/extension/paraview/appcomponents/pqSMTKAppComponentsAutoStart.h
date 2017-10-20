//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#pragma once

#include "smtk/extension/paraview/appcomponents/PluginExports.h"

#include <QObject>

class SMTKPQCOMPONENTSPLUGIN_EXPORT pqSMTKAppComponentsAutoStart : public QObject
{
  Q_OBJECT
  using Superclass = QObject;

public:
  pqSMTKAppComponentsAutoStart(QObject* parent = nullptr);
  ~pqSMTKAppComponentsAutoStart() override;

  void startup();
  void shutdown();

private:
  Q_DISABLE_COPY(pqSMTKAppComponentsAutoStart);
};
