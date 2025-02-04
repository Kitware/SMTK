//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/ReferenceItemDefinition.h"
#include "smtk/attribute/Registrar.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/io/AttributeReader.h"
#include "smtk/io/Logger.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/Operation.h"
#include "smtk/operation/XMLOperation.h"
#include "smtk/plugin/Registry.h"
#include "smtk/project/Manager.h"
#include "smtk/project/Project.h"
#include "smtk/project/Registrar.h"
#include "smtk/project/operators/Create.h"
#include "smtk/resource/Manager.h"

#include "smtk/common/testing/cxx/helpers.h"

#include <iostream>

#define ATT_ROLE_NAME "attributes"
#define PROJECT_TYPE "foo"

// This test verifies that projects can be instantiated outside of the
// SMTK resource manager.

namespace
{
const int OP_SUCCEEDED = static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED);

class CreateProjectOp : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(CreateProjectOp);
  smtkCreateMacro(CreateProjectOp);
  smtkSharedFromThisMacro(smtk::operation::Operation);

  CreateProjectOp() = default;
  ~CreateProjectOp() override = default;

  bool ableToOperate() override
  {
    if (m_projManager == nullptr)
    {
      smtkErrorMacro(this->log(), "Project Manager not set");
      return false;
    }
    return smtk::operation::XMLOperation::ableToOperate();
  }

  Result operateInternal() override
  {
    auto project = m_projManager->create(PROJECT_TYPE);
    if (this->managers())
    {
      project->resources().setManager(this->managers()->get<smtk::resource::Manager::Ptr>());
      project->operations().setManager(this->managers()->get<smtk::operation::Manager::Ptr>());
    }

    auto attRes = m_projManager->resourceManager()->create<smtk::attribute::Resource>();
    project->resources().add(attRes, ATT_ROLE_NAME);

    auto result = this->createResult(Outcome::SUCCEEDED);
    auto res = result->findResource("resourcesCreated");
    res->setValue(project);
    return result;
  }

  const char* xmlDescription() const override;

  smtk::project::Manager::Ptr m_projManager;
};

const char CreateProjectOpXML[] = R"xml(
<?xml version="1.0" encoding="utf-8" ?>
<SMTK_AttributeResource Version="3">
  <Definitions>
    <AttDef Type="operation" Label="operation" Abstract="True">
      <ItemDefinitions>
        <Int Name="debug level" Optional="True">
          <DefaultValue>0</DefaultValue>
        </Int>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="result" Abstract="True">
      <ItemDefinitions>
        <Int Name="outcome" Label="outcome" Optional="False" NumberOfRequiredValues="1">
        </Int>
        <String Name="log" Optional="True" NumberOfRequiredValues="0" Extensible="True">
        </String>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="create-project-op" Label="A Test Operation" BaseType="operation">
      <ItemDefinitions>
        <Int Name="my int" Optional="False">
          <DefaultValue>0</DefaultValue>
        </Int>
      </ItemDefinitions>
    </AttDef>
    <AttDef Type="result(create-project-op)" BaseType="result">
      <ItemDefinitions>
        <Resource Name="resourcesCreated" HoldReference="true">
          <Accepts>
            <Resource Name="smtk::project::Project"/>
          </Accepts>
        </Resource>
      </ItemDefinitions>
    </AttDef>
  </Definitions>
</SMTK_AttributeResource>)xml";

const char* CreateProjectOp::xmlDescription() const
{
  return CreateProjectOpXML;
}

} // namespace

int TestProjectLifeCycle(int /*unused*/, char** const /*unused*/)
{
  // Create managers
  smtk::common::Managers::Ptr managers = smtk::common::Managers::create();
  smtk::resource::ManagerPtr resManager = smtk::resource::Manager::create();
  smtk::operation::ManagerPtr opManager = smtk::operation::Manager::create();
  auto attributeRegistry =
    smtk::plugin::addToManagers<smtk::attribute::Registrar>(resManager, opManager);
  managers->insertOrAssign(resManager);
  managers->insertOrAssign(opManager);
  opManager->setManagers(managers);

#if 0
  // This line changes behavior such that projects ARE stored in resource manager
  opManager->registerResourceManager(resManager);
#endif

  opManager->registerOperation<CreateProjectOp>("CreateProjectOp");

  smtk::project::ManagerPtr projManager = smtk::project::Manager::create(resManager, opManager);
  auto projectRegistry = smtk::plugin::addToManagers<smtk::project::Registrar>(projManager);
  projManager->registerProject(PROJECT_TYPE);
  managers->insertOrAssign(projManager);

  smtkTest(resManager->empty(), "resource manager size is " << resManager->size());

  smtk::resource::Resource::Ptr resource;
  smtk::project::Project::Ptr project;
  {
    std::cout << "Creating project" << std::endl;

    auto createOp = opManager->create<CreateProjectOp>();
    smtkTest(createOp != nullptr, "create operation not created");
    createOp->m_projManager = projManager; // back door

    auto result = createOp->operate();
    int outcome = result->findInt("outcome")->value();
    smtkTest(outcome == OP_SUCCEEDED, "create operation failed");

    auto res = result->findResource("resourcesCreated");
    project = std::dynamic_pointer_cast<smtk::project::Project>(res->value(0));
    smtkTest(res->value() != nullptr, "project not created");
  }

  {
    // Is project contained by resource manager?
    auto res = resManager->get<smtk::project::Project>(project->id());
    smtkTest(res == nullptr, "resource manager contains project")
      smtkTest(resManager->size() == 1, "resource manager size not 1");

    auto attRes = project->resources().findByRole(ATT_ROLE_NAME);
    smtkTest(!attRes.empty(), "project missing attribute resource with role: " << ATT_ROLE_NAME);

    // Release the project, check that resource manager unchanged
    std::cout << "Removing project" << std::endl;
    projManager->remove(project);
    smtkTest(resManager->size() == 1, "resource manager size not 1");

    // And remove the attribute resource
    resManager->remove(*(attRes.begin()));
    smtkTest(resManager->empty(), "resource manager not empty");
  }

  std::cout << "Finis" << std::endl;
  return 0;
}
