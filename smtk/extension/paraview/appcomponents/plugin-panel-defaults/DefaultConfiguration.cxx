//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/plugin-panel-defaults/DefaultConfiguration.h"

#include "smtk/extension/paraview/appcomponents/pqSMTKOperationToolboxPanel.h"
#include "smtk/view/Configuration.h"
#include "smtk/view/Information.h"
#include "smtk/view/json/jsonView.h"

#include "smtk/io/Logger.h"

DefaultConfiguration::DefaultConfiguration(QObject* parent)
  : QObject(parent)
{
}

smtk::view::Information DefaultConfiguration::panelConfiguration(const QWidget* panel)
{
  smtk::view::Information result;
  if (const auto* toolbox = dynamic_cast<const pqSMTKOperationToolboxPanel*>(panel))
  {
    (void)toolbox;
    nlohmann::json jsonConfig = {
      { "Name", "Operations" },
      { "Type", "qtOperationPalette" },
      { "Component",
        { { "Name", "Details" },
          { "Attributes", { { "SearchBar", true }, { "Title", "Tools" } } },
          { "Children",
            { { { "Name", "Model" }, { "Attributes", { { "Autorun", "true" } } } } } } } }
    };
    std::shared_ptr<smtk::view::Configuration> viewConfig = jsonConfig;
    result.insert_or_assign(viewConfig);
  }
  else
  {
    smtkWarningMacro(
      smtk::io::Logger::instance(),
      "Unknown panel named \"" << panel->objectName().toStdString() << "\"\n");
  }
  return result;
}
