//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/ApplicationConfiguration.h"

#include "pqApplicationCore.h"
#include "pqInterfaceTracker.h"

namespace smtk
{
namespace paraview
{

namespace
{

QMetaObject::Connection g_trackerConnection;

} // anonymous namespace

std::vector<ApplicationConfiguration::ConfigurationObserver>
  ApplicationConfiguration::s_uiComponents;

bool ApplicationConfiguration::notify(ConfigurationObserver observer)
{
  // Queue the observer in the list of UI components to be initialized.
  s_uiComponents.push_back(observer);
  // Try to find an existing application configuration object:
  auto* core = pqApplicationCore::instance();
  if (core)
  {
    auto* tracker = core->interfaceTracker();
    auto interfaces = tracker->interfaces<ApplicationConfiguration*>();
    if (!interfaces.empty())
    {
      for (auto& iface : interfaces)
      {
        if (ApplicationConfiguration::found(dynamic_cast<QObject*>(iface)))
        {
          return true;
        }
      }
    }
    // Queue an observer if we don't find an interface... only queue it once.
    if (!g_trackerConnection)
    {
      g_trackerConnection =
        QObject::connect(tracker, &pqInterfaceTracker::interfaceRegistered, [](QObject* iface) {
          ApplicationConfiguration::found(iface);
        });
    }
  }
  return false;
}

bool ApplicationConfiguration::found(QObject* iface)
{
  auto* configurator = dynamic_cast<ApplicationConfiguration*>(iface);
  if (!configurator)
  {
    return false;
  }
  for (auto& observer : s_uiComponents)
  {
    observer(*configurator);
  }
  s_uiComponents.clear();
  return true;
}

} // namespace paraview
} // namespace smtk
