//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/project/json/jsonProject.h"

#include "smtk/resource/json/Helper.h"
#include "smtk/resource/json/jsonResource.h"

#include "smtk/project/json/jsonOperationFactory.h"
#include "smtk/project/json/jsonResourceContainer.h"
#include "smtk/task/json/Helper.h"
#include "smtk/task/json/jsonFillOutAttributes.h"
#include "smtk/task/json/jsonGatherResources.h"
#include "smtk/task/json/jsonManager.h"
#include "smtk/task/json/jsonTask.h"

// Define how projects are serialized.
namespace smtk
{
namespace project
{
void to_json(json& jj, const ProjectPtr& project)
{
  smtk::resource::to_json(jj, std::static_pointer_cast<smtk::resource::Resource>(project));

  to_json(jj["resources"], project->resources(), project);
  to_json(jj["operations"], project->operations());

  jj["task_manager"] = project->taskManager();
  jj["conceptual_version"] = project->version();
}

void from_json(const json& jj, ProjectPtr& project)
{
  if (!project)
  {
    return;
  }

  smtk::resource::ResourcePtr tmp = std::static_pointer_cast<smtk::resource::Resource>(project);
  smtk::resource::from_json(jj, tmp);

  from_json(jj["resources"], project->resources(), project);
  from_json(jj["operations"], project->operations());
  auto it = jj.find("task_manager");
  if (it != jj.end())
  {
    from_json(*it, project->taskManager());
  }

  project->setVersion(jj["conceptual_version"]);
}
} // namespace project
} // namespace smtk
