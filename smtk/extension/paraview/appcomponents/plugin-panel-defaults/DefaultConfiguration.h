//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_DefaultConfiguration_h
#define smtk_extension_paraview_appcomponents_DefaultConfiguration_h

#include "smtk/extension/paraview/appcomponents/ApplicationConfiguration.h"

/**\brief A default application configuration for non-custom applications.
  *
  * Normally, applications will provide their own subclass of
  * ApplicationConfiguration. If using SMTK plugins on their own
  * with ParaView or ModelBuilder, the plugin containing this
  * object will provide some defaults.
  */
class DefaultConfiguration
  : public QObject
  , public smtk::paraview::ApplicationConfiguration

{
  Q_OBJECT
  Q_INTERFACES(smtk::paraview::ApplicationConfiguration)

public:
  DefaultConfiguration(QObject* parent);
  ~DefaultConfiguration() override = default;

  /**\brief Provide configuration information for panels.
    *
    */
  smtk::view::Information panelConfiguration(const QWidget* panel) override;
};

#endif // smtk_extension_paraview_appcomponents_DefaultConfiguration_h
