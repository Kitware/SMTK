//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_project_Utility_h
#define smtk_extension_paraview_project_Utility_h
/*!\file Utility.h
 * Functions provided here are for working with projects and tasks
 * in ParaView's user interface.
 *
 * The methods here can be used to fetch ParaView pipeline objects and the
 * blocks within them that are relevant to a particular SMTK component or
 * resource.
 */

#include "smtk/extension/paraview/project/smtkPQProjectExtModule.h"

#include "smtk/project/Observer.h"
#include "smtk/task/Active.h"
#include "smtk/task/Manager.h"
#include "smtk/task/Task.h"

#include "smtk/PublicPointerDefs.h"

class pqDataRepresentation;

namespace smtk
{
namespace paraview
{

using RepresentationObjectMap =
  std::map<pqDataRepresentation*, std::unordered_set<smtk::resource::PersistentObject*>>;

/// Given a map from resources to persistent objects, return a map from
/// representations tied to the resource keys to persistent object values.
///
/// This is intended for use by user interface code that needs to adjust the visibility of
/// components and/or resources.
RepresentationObjectMap representationsOfObjects(
  const smtk::task::Manager::ResourceObjectMap& resourcesToObjects);

/// Return a list of representations that are relevant to \a spec and \a task.
///
/// These representations will correspond to a resources that match
/// a directive above because (a) they are part of the project owning the
/// task manager or (b) they are on input ports of the task referenced by
/// the \a spec.
RepresentationObjectMap relevantRepresentations(
  const nlohmann::json& spec,
  smtk::task::Manager* taskMgr,
  smtk::task::Task* task);

} // namespace paraview
} // namespace smtk

#endif // smtk_extension_paraview_project_Utility_h
