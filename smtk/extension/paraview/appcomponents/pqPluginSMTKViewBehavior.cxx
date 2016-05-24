//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/pqPluginSMTKViewBehavior.h"

#include "smtk/extension/qt/qtSMTKUtilities.h"
#include "pqApplicationCore.h"
#include "pqInterfaceTracker.h"

//-----------------------------------------------------------------------------
pqPluginSMTKViewBehavior::pqPluginSMTKViewBehavior(QObject* p)
  : Superclass(p)
{
  pqInterfaceTracker* pm = pqApplicationCore::instance()->interfaceTracker();
  QObject::connect(pm, SIGNAL(interfaceRegistered(QObject*)),
    this, SLOT(addPluginInterface(QObject*)));
  foreach (QObject* iface, pm->interfaces())
    {
    this->addPluginInterface(iface);
    }
}

//-----------------------------------------------------------------------------
void pqPluginSMTKViewBehavior::addPluginInterface(QObject* iface)
{
  smtk::extension::qtViewInterface* svi = qobject_cast<
    smtk::extension::qtViewInterface*>(iface);
  if (!svi)
    {
    return;
    }
  qtSMTKUtilities::registerViewConstructor(svi->viewName().toStdString(),
                                           svi->viewConstructor());
}
