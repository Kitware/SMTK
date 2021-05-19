//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#define SMTK_DEPRECATION_LEVEL 2105

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/Registrar.h"
#include "smtk/attribute/Resource.h"
#include "smtk/attribute/ResourceItem.h"
#include "smtk/io/AttributeReader.h"
#include "smtk/io/Logger.h"
#include "smtk/operation/Manager.h"
#include "smtk/operation/Operation.h"
#include "smtk/operation/XMLOperation.h"
#include "smtk/project/Manager.h"
#include "smtk/project/Project.h"
#include "smtk/project/Registrar.h"
#include "smtk/project/operators/Create.h"

#include "smtk/common/testing/cxx/helpers.h"

#include <iostream>

#define ATT_ROLE_NAME "attributes"
#define OP_NAME "create-project-op"
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

    auto attRes = m_projManager->resourceManager()->create<smtk::attribute::Resource>();
    project->resources().add(attRes, ATT_ROLE_NAME);

    auto result = this->createResult(Outcome::SUCCEEDED);
    result->findResource("resource")->setValue(project);
    return result;
  }

  const char* xmlDescription() const override;

  smtk::project::Manager::Ptr m_projManager;
};

const char CreateProjectOpXML[] =
  "<?xml version=\"1.0\" encoding=\"utf-8\" ?>"
  "<SMTK_AttributeSystem Version=\"2\">"
  "  <Definitions>"
  "    <AttDef Type=\"operation\" Label=\"operation\" Abstract=\"True\">"
  "      <ItemDefinitions>"
  "        <Int Name=\"debug level\" Optional=\"True\">"
  "          <DefaultValue>0</DefaultValue>"
  "        </Int>"
  "      </ItemDefinitions>"
  "    </AttDef>"
  "    <AttDef Type=\"result\" Abstract=\"True\">"
  "      <ItemDefinitions>"
  "        <Int Name=\"outcome\" Label=\"outcome\" Optional=\"False\" NumberOfRequiredValues=\"1\">"
  "        </Int>"
  "        <String Name=\"log\" Optional=\"True\" NumberOfRequiredValues=\"0\" Extensible=\"True\">"
  "        </String>"
  "      </ItemDefinitions>"
  "    </AttDef>"
  "    <AttDef Type=\"" OP_NAME "\" Label=\"A Test Operation\" BaseType=\"operation\">"
  "      <ItemDefinitions>"
  "        <Int Name=\"my int\" Optional=\"False\">"
  "          <DefaultValue>0</DefaultValue>"
  "        </Int>"
  "      </ItemDefinitions>"
  "    </AttDef>"
  "    <AttDef Type=\"result(test op)\" BaseType=\"result\">"
  "      <ItemDefinitions>"
  "        <Resource Name=\"resource\" HoldReference=\"true\">"
  "          <Accepts>"
  "            <Resource Name=\"smtk::project::Project\"/>"
  "          </Accepts>"
  "        </Resource>"
  "      </ItemDefinitions>"
  "    </AttDef>"
  "  </Definitions>"
  "</SMTK_AttributeSystem>";

const char* CreateProjectOp::xmlDescription() const
{
  return CreateProjectOpXML;
}

} // namespace

int TestProjectLifeCycle_Deprecated(int /*unused*/, char** const /*unused*/)
{
  // Create managers
  smtk::resource::ManagerPtr resManager = smtk::resource::Manager::create();
  smtk::attribute::Registrar::registerTo(resManager);

  smtk::operation::ManagerPtr opManager = smtk::operation::Manager::create();
  smtk::attribute::Registrar::registerTo(opManager);

#if 0
  // This line changes behavior such that projects ARE stored in resource manager
  opManager->registerResourceManager(resManager);
#endif

  opManager->registerOperation<CreateProjectOp>("CreateProjectOp");

  smtk::project::ManagerPtr projManager = smtk::project::Manager::create(resManager, opManager);
  smtk::project::Registrar::registerTo(projManager);
  projManager->registerProject(PROJECT_TYPE);

  smtkTest(resManager->empty(), "resource manager size is " << resManager->size());

  smtk::resource::Resource::Ptr resource;
  smtk::project::Project::Ptr project;
  {
    std::cout << "Creating project" << std::endl;

    auto createOp = opManager->create<CreateProjectOp>();
    smtkTest(createOp != nullptr, "create operation not created") createOp->m_projManager =
      projManager; // back door

    auto result = createOp->operate();
    int outcome = result->findInt("outcome")->value();
    smtkTest(outcome == OP_SUCCEEDED, "create operation failed");

    auto res = result->findResource("resource")->value();
    project = std::dynamic_pointer_cast<smtk::project::Project>(res);
    smtkTest(res != nullptr, "project not created");
  }

  {
    // Is project contained by resource manager?
    auto res = resManager->get<smtk::project::Project>(project->id());
    smtkTest(res == nullptr, "resource manager contains project")
      smtkTest(resManager->size() == 1, "resource manager size not 1");

    auto attRes = project->resources().getByRole(ATT_ROLE_NAME);
    smtkTest(attRes != nullptr, "project missing attribute resource");

    // Release the project, check that resource manager unchanged
    std::cout << "Removing project" << std::endl;
    projManager->remove(project);
    smtkTest(resManager->size() == 1, "resource manager size not 1");

    // And remove the attribute resource
    resManager->remove(attRes);
    smtkTest(resManager->empty(), "resource manager not empty");
  }

  std::cout << "Finis" << std::endl;
  return 0;
}
