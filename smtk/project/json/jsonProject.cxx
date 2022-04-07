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

#include "smtk/resource/json/jsonResource.h"

#include "smtk/project/json/jsonOperationFactory.h"
#include "smtk/project/json/jsonResourceContainer.h"

// Define how projects are serialized.
namespace smtk
{
namespace project
{
void to_json(json& j, const ProjectPtr& project)
{
  smtk::resource::to_json(j, std::static_pointer_cast<smtk::resource::Resource>(project));

  to_json(j["resources"], project->resources(), project);
  to_json(j["operations"], project->operations());

  j["conceptual_version"] = project->version();
}

void from_json(const json& j, ProjectPtr& project)
{
  if (!project)
  {
    return;
  }

  smtk::resource::ResourcePtr tmp = std::static_pointer_cast<smtk::resource::Resource>(project);
  smtk::resource::from_json(j, tmp);

  from_json(j["resources"], project->resources(), project);
  from_json(j["operations"], project->operations());

  project->setVersion(j["conceptual_version"]);
}
} // namespace project
} // namespace smtk
