//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/qt/qtDescriptivePhraseDelegate.h"
#include "smtk/extension/qt/qtDescriptivePhraseModel.h"

#include "smtk/view/Configuration.h"
#include "smtk/view/Manager.h"
#include "smtk/view/PhraseModel.h"
#include "smtk/view/PhraseModelFactory.h"
#include "smtk/view/Registrar.h"
#include "smtk/view/ResourcePhraseModel.h"
#include "smtk/view/json/jsonView.h"

#include "smtk/model/Registrar.h"
#include "smtk/model/Resource.h"
#include "smtk/model/json/jsonResource.h"

#include "smtk/attribute/Registrar.h"

#include "smtk/common/TypeContainer.h"

#include "smtk/extension/qt/examples/cxx/ModelBrowser.h"

#include "smtk/common/testing/cxx/helpers.h"
#include "smtk/model/testing/cxx/helpers.h"

#include <QApplication>
#include <QHeaderView>
#include <QTimer>
#include <QTreeView>

#include <fstream>
#include <iomanip>
#include <iostream>

#include <cstdlib>

using namespace std;

int main(int argc, char* argv[])
{
  QApplication app(argc, argv);
  const char* filename = argc > 1 ? argv[1] : "smtkModel.json";
  bool debug = (argc > 2 && argv[2][0] != '0');
  const char* configname = argc > 3 ? argv[3] : nullptr;

  std::ifstream file(filename);
  if (!file.good())
  {
    cout << "Could not open file \"" << filename << "\".\n\n"
         << "Usage:\n  " << argv[0] << " [[[filename] debug] viewconf]\n"
         << "where\n"
         << "  filename   is the path to a JSON model.\n"
         << "  debug      is any character other than '0', indicating a debug session.\n\n"
         << "  viewconf   is the path to a JSON view-configuration file.\n"
         << "             If no config is provided, a default is used.\n";
    return 1;
  }

  auto operationManager = smtk::operation::Manager::create();
  auto resourceManager = smtk::resource::Manager::create();
  auto viewManager = smtk::view::Manager::create();
  smtk::view::Registrar::registerTo(viewManager);
  smtk::model::Registrar::registerTo(resourceManager);
  smtk::attribute::Registrar::registerTo(resourceManager);

  nlohmann::json jconfig;
  if (configname)
  {
    std::ifstream configFile(configname);
    if (!configFile.good())
    {
      cout << "Could not open configuration file \"" << configname << "\".\n\n";
      return 1;
    }
    std::string json_str(
      (std::istreambuf_iterator<char>(configFile)), (std::istreambuf_iterator<char>()));
    jconfig = nlohmann::json::parse(json_str);
  }
  else
  {
    jconfig = {
      { "Name", "Test" },
      { "Type", "smtk::view::ResourcePhraseModel" },
      { "Component",
        { { "Name", "Details" },
          { "Type", "smtk::view::ResourcePhraseModel" },
          { "Attributes", { { "TopLevel", true }, { "Title", "Resources" } } },
          { "Children",
            { { { "Name", "PhraseModel" },
                { "Attributes", { { "Type", "smtk::view::ResourcePhraseModel" } } },
                { "Children",
                  { { { "Name", "SubphraseGenerator" },
                      { "Attributes", { { "Type", "default" } } } },
                    { { "Name", "Badges" },
                      { "Children",
                        {
                          { { "Name", "Badge" },
                            { "Attributes", { { "Type", "smtk::view::ObjectIconBadge" } } } },
                        } } },
                    { { "Name", "Accepts" },
                      { "Children",
                        {
                          { { "Name", "Resource" },
                            { "Attributes",
                              { { "Name", "smtk::resource::Resource" }, { "Filter", "any" } } } },
                        } } } } } } } } } }
    };
  }

  std::string json_str((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
  nlohmann::json json = nlohmann::json::parse(json_str);
  auto model = smtk::model::Resource::create();
  smtk::model::from_json(json, model);

  if (!model)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "Did not read a model.");
    return 1;
  }
  model->assignDefaultNames();
  resourceManager->add(model);
  std::cout << "Read a " << model->typeName() << "\n";

  auto qmodel = new smtk::extension::qtDescriptivePhraseModel;
  auto qdelegate = new smtk::extension::qtDescriptivePhraseDelegate;
  qdelegate->setTitleFontSize(12);
  qdelegate->setTitleFontWeight(2);
  qdelegate->setSubtitleFontSize(10);
  qdelegate->setSubtitleFontWeight(1);
  ModelBrowser* qview = new ModelBrowser;
  smtk::view::ConfigurationPtr config = jconfig;
  auto phraseModel = viewManager->phraseModelFactory().createFromConfiguration(config.get());
  phraseModel->addSource({ resourceManager, operationManager, viewManager });
  qmodel->setPhraseModel(phraseModel);
  qview->setup(resourceManager, qmodel, qdelegate);

  // Enable user sorting.
  qview->tree()->setSortingEnabled(true);
  qview->show();

  // If we are in debug mode, we just want to ensure that the application
  // successfully executes.
  if (debug)
  {
    QTimer::singleShot(1000, &app, SLOT(quit()));
  }
  int status = QApplication::exec();

  delete qview;
  delete qmodel;
  delete qdelegate;

  return status;
}
