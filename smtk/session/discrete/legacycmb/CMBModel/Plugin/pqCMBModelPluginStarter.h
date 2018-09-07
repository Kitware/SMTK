//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __pqCMBModelPluginStarter_h
#define __pqCMBModelPluginStarter_h

#include "cmbSystemConfig.h"
#include <QObject>

class pqCMBModelPluginStarter : public QObject
{
  Q_OBJECT
  typedef QObject Superclass;

public:
  pqCMBModelPluginStarter(QObject* p = 0);
  ~pqCMBModelPluginStarter();

  // Callback for shutdown.
  void onShutdown();

  // Callback for startup.
  void onStartup();

private:
  pqCMBModelPluginStarter(const pqCMBModelPluginStarter&); // Not implemented.
  void operator=(const pqCMBModelPluginStarter&);          // Not implemented.
};

#endif
