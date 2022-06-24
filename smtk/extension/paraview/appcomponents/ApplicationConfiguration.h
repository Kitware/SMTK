//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_ApplicationConfiguration_h
#define smtk_extension_paraview_appcomponents_ApplicationConfiguration_h

#include "smtk/extension/paraview/appcomponents/smtkPQComponentsExtModule.h"

#include "smtk/view/Information.h"

#include <QObject>

#include <functional>
#include <vector>

class QWidget;

namespace smtk
{
namespace paraview
{

/**\brief An interface by which applications can configure SMTK UI components.
  *
  * SMTK UI components (not resource::Component, but panels, toolbars, etc.)
  * should use the ApplicationConfiguration::notify() function to provide an
  * Observer that will be invoked either immediately or when a plugin is loaded
  * that provides an instance of ApplicationConfiguration.
  *
  * Currently this interface only supports panels, but API may be added for
  * other components as needed.
  */
class SMTKPQCOMPONENTSEXT_EXPORT ApplicationConfiguration
{
public:
  using ConfigurationObserver = std::function<void(ApplicationConfiguration&)>;

  /**\brief Wait until a configuration provider is registered and invoke an observer.
    *
    * This monitors the `pqApplicationCore::instance()`'s interface tracker until an
    * instance of ApplicationConfiguration becomes available.
    * (It may be available immediately via `pqInterfaceTracker::interfaces()`
    * or later upon the `pqInterfaceTracker::interfaceRegistered()` signal.)
    * Once found, the observer is invoked so it can configure components as needed by
    * calling methods on the interface.
    *
    * This method returns true if the configuration instance was invoked immediately
    * and false if a signal is queued to fire should a configuration instance be
    * registered later.
    */
  static bool notify(ConfigurationObserver observer);

  /**\brief Method for applications to provide configuration information for panels.
    *
    * Applications should dynamic-cast the \a panel to each of the panel types that
    * it supports and return view information.
    */
  virtual smtk::view::Information panelConfiguration(const QWidget* panel) = 0;

protected:
  /// A "slot" to be invoked when an interface implementation is available.
  static bool found(QObject* iface);

  /// Observers waiting for an interface to be invoked.
  static std::vector<ConfigurationObserver> s_uiComponents;
};

} // namespace paraview
} // namespace smtk

Q_DECLARE_INTERFACE(
  smtk::paraview::ApplicationConfiguration,
  "com.kitware.smtk.ApplicationConfiguration");

#endif // smtk_extension_paraview_appcomponents_ApplicationConfiguration_h
