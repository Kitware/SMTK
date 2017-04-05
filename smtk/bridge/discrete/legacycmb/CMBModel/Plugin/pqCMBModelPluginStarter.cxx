//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "pqCMBModelPluginStarter.h"

// Server Manager Includes.
#include "vtkClientServerInterpreter.h"
#include "vtkClientServerInterpreterInitializer.h"
#include "vtkProcessModule.h"

// Qt Includes.
//#include <QtDebug>

// ParaView Includes.

// ClientServer wrapper initialization functions.
extern "C" void vtkCmbDiscreteModelCS_Initialize(vtkClientServerInterpreter*);

pqCMBModelPluginStarter::pqCMBModelPluginStarter(QObject* p/*=0*/)
  : QObject(p)
{
}

pqCMBModelPluginStarter::~pqCMBModelPluginStarter()
{
}

void pqCMBModelPluginStarter::onStartup()
{
    // FIXME SEB Don't need that anymore // vtkCmbDiscreteModelCS_Initialize(vtkClientServerInterpreterInitializer::GetInitializer());
}

void pqCMBModelPluginStarter::onShutdown()
{
  //qWarning() << "Message from pqCMBPluginStarter: Application Shutting down";
}
