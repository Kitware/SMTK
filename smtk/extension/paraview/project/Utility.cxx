//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/project/Utility.h"

#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKResource.h"

#include "smtk/task/Manager.h"

#include "pqDataRepresentation.h"

namespace smtk
{
namespace paraview
{

RepresentationObjectMap representationsOfObjects(
  const smtk::task::Manager::ResourceObjectMap& resourcesToObjects)
{
  // Filter representations by the \a spec.
  RepresentationObjectMap representations;

  auto* behavior = pqSMTKBehavior::instance();
  for (const auto& entry : resourcesToObjects)
  {
    smtk::resource::Resource* resource = entry.first;
    if (!resource)
    {
      continue;
    }

    QPointer<pqSMTKResource> pvrsrc = behavior->getPVResource(resource->shared_from_this());
    if (!pvrsrc)
    {
      continue;
    }

    for (const auto& view : pvrsrc->getViews())
    {
      for (const auto& representation : pvrsrc->getRepresentations(view))
      {
        for (const auto& object : entry.second)
        {
          representations[representation].insert(object);
        }
      }
    }
  }

  return representations;
}

RepresentationObjectMap relevantRepresentations(
  const nlohmann::json& spec,
  smtk::task::Manager* taskMgr,
  smtk::task::Task* task)
{
  auto objectMap = taskMgr->workflowObjects(spec, task);
  auto representations = representationsOfObjects(objectMap);
  return representations;
}

} // namespace paraview
} // namespace smtk
