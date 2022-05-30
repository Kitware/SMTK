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

#include "smtk/plugin/Registry.h"

#include "smtk/project/examples/cxx/ProjectBrowser.h"

#include "smtk/project/Manager.h"
#include "smtk/project/Project.h"
#include "smtk/project/Registrar.h"

#include "smtk/common/testing/cxx/helpers.h"
#include "smtk/model/testing/cxx/helpers.h"

#include <QApplication>
#include <QHeaderView>
#include <QTimer>
#include <QTreeView>

#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>

using namespace std;

namespace
{
class MyResource : public smtk::resource::DerivedFrom<MyResource, smtk::resource::Resource>
{
public:
  smtkTypeMacro(MyResource);
  smtkCreateMacro(MyResource);
  smtkSharedFromThisMacro(smtk::resource::PersistentObject);

  smtk::resource::ComponentPtr find(const smtk::common::UUID& /*compId*/) const override
  {
    return smtk::resource::ComponentPtr();
  }

  std::function<bool(const smtk::resource::Component&)> queryOperation(
    const std::string& /*unused*/) const override
  {
    return [](const smtk::resource::Component& /*unused*/) { return true; };
  }

  void visit(smtk::resource::Component::Visitor& /*v*/) const override {}

protected:
  MyResource() = default;
};

struct Registrar
{
  static void registerTo(const smtk::resource::Manager::Ptr& resourceManager)
  {
    resourceManager->registerResource<MyResource>();
  }

  static void unregisterFrom(const smtk::resource::Manager::Ptr& resourceManager)
  {
    resourceManager->unregisterResource<MyResource>();
  }

  static void registerTo(const smtk::project::Manager::Ptr& projectManager)
  {
    projectManager->registerProject("MyProject", { "MyResource" }, {});
  }

  static void unregisterFrom(const smtk::project::Manager::Ptr& projectManager)
  {
    (void)projectManager;
    // projectManager->unregisterProject("MyProject");
  }
};
} // namespace

int main(int argc, char* argv[])
{
  QApplication app(argc, argv);
  const char* configname = argc > 1 ? argv[1] : nullptr;

  if (argc > 1 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0))
  {
    std::cout << "Usage:\n  " << argv[0] << " [viewconf]\n"
              << "where\n"
              << "  viewconf   is the path to a JSON view-configuration file.\n"
              << "             If no config is provided, a default is used.\n";
    return 1;
  }

  // Create a resource manager
  smtk::resource::ManagerPtr resourceManager = smtk::resource::Manager::create();

  // Create an operation manager
  smtk::operation::ManagerPtr operationManager = smtk::operation::Manager::create();

  // Register project resources and operations with their managers
  auto projectRegistry =
    smtk::plugin::addToManagers<smtk::project::Registrar>(resourceManager, operationManager);

  // Create a project manager
  smtk::project::ManagerPtr projectManager =
    smtk::project::Manager::create(resourceManager, operationManager);

  // Register MyResource our not-so-custom Project with the manager.
  auto registry = smtk::plugin::addToManagers<::Registrar>(resourceManager, projectManager);

  // Create an instance of smtk::project::Project
  auto project = projectManager->create("MyProject");

  // Create a resource
  auto myResource = resourceManager->create<MyResource>();
  myResource->setName("My Resource");

  // Add the resource to the project
  project->resources().add(myResource);

  auto viewManager = smtk::view::Manager::create();
  auto viewRegistry = smtk::plugin::addToManagers<smtk::view::Registrar>(viewManager);
  auto modelRegistry = smtk::plugin::addToManagers<smtk::model::Registrar>(resourceManager);
  auto attributeRegistry = smtk::plugin::addToManagers<smtk::attribute::Registrar>(resourceManager);
  auto projectViewRegistry = smtk::plugin::addToManagers<smtk::project::Registrar>(viewManager);

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
      { "Type", "smtk::project::view::PhraseModel" },
      { "Component",
        { { "Name", "Details" },
          { "Type", "smtk::project::view::PhraseModel" },
          { "Attributes", { { "TopLevel", true }, { "Title", "Projects" } } },
          { "Children",
            { { { "Name", "PhraseModel" },
                { "Attributes", { { "Type", "smtk::project::view::PhraseModel" } } },
                { "Children",
                  { { { "Name", "SubphraseGenerator" },
                      { "Attributes", { { "Type", "smtk::project::view::SubphraseGenerator" } } } },
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

  auto* qmodel = new smtk::extension::qtDescriptivePhraseModel;
  qmodel->setColumnName("Project");
  auto* qdelegate = new smtk::extension::qtDescriptivePhraseDelegate;
  qdelegate->setTitleFontSize(12);
  qdelegate->setTitleFontWeight(2);
  qdelegate->setSubtitleFontSize(10);
  qdelegate->setSubtitleFontWeight(1);
  ProjectBrowser* qview = new ProjectBrowser;
  smtk::view::ConfigurationPtr config = jconfig;
  auto phraseModel = viewManager->phraseModelFactory().createFromConfiguration(config.get());
  phraseModel->addSource({ resourceManager, operationManager, viewManager, projectManager });
  qmodel->setPhraseModel(phraseModel);
  qview->setup(resourceManager, qmodel, qdelegate);

  // Enable user sorting.
  qview->tree()->setSortingEnabled(true);
  qview->show();

  int status = QApplication::exec();

  delete qview;
  delete qmodel;
  delete qdelegate;

  return status;
}
