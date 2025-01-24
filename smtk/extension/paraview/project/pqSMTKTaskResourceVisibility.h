//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_pqSMTKTaskResourceVisibility_h
#define smtk_extension_paraview_appcomponents_pqSMTKTaskResourceVisibility_h

#include "smtk/extension/paraview/project/smtkPQProjectExtModule.h"

#include "smtk/project/Observer.h"
#include "smtk/task/Active.h"
#include "smtk/task/Task.h"

#include "smtk/PublicPointerDefs.h"

#include "pqReaction.h"

#include "smtk/extension/paraview/appcomponents/pqQtKeywordWrapping.h"

#include <QObject>

class pqRepresentation;
class pqSMTKWrapper;
class pqServer;

/**\brief Let the active task control the visibility of resources/components.
  *
  * When a project is loaded, this class monitors the task manager for changes
  * to the active task. When a change is detected, the style of the previous
  * and upcoming tasks are inspected to identify resources and components to
  * hide and show.
  *
  * When a style entry (in the task-manager's "styles" section) for the active
  * task includes a "3d-view" directive, it is used to update the visibility
  * of resources and components as the task transitions to/from being the
  * active task.
  *
  * The "3d-view" dictionary may contain "color-by", "hide", and/or "show" directives.
  * Each of these should be an array of dictionaries, each of which must
  * have an "event" (specifying "activated" or "deactivated") and may
  * have following:
  *
  * + "source": specifying how to obtain objects to control; and
  * + "filter": specifying how to choose resources and/or components by role and/or type.
  *
  * The "source" must be a dictionary holding
  *
  * + "type" (with a value of "project resources" or "active task port", assumed to be
  *   "project resources" if no source is provided); and
  * + "port" (with the name of a port on the active task) if the type is "active task port".
  * + "roles" (with the name of a role for data on the port) if the type is "active task port".
  *
  * The "filter" must be an dictionary holding:
  *
  * + "resources", an optional array of accepted resource type-names or "*". If not present, "*" is assumed.
  * + "components", an optional array of accepted component-filters or "*". If not present, only
  *
  * When a task is deactivated and no new task is activated at the same time,
  * (1) if the task-path is empty (i.e., the top-level tasks are showing), then the default
  *     styles are applied; or
  * (2) if the task-path is non-empty, the right-most task's style is applied.
  *
  * If you wish to provide a project default (when no tasks are active but a task-manager
  * is present), include a style for the tag named "default".
  *
  * An example is:
  * ```json
  * "styles": {
  *   "default": { "3d-view": { "color-by": { "mode": "none" } } },
  *   "example": {
  *     "3d-view": {
  *       "color-by": { "mode": "attribute-association", "definition": "BoundaryCondition",
  *         "event": "activated" },
  *       "hide": [
  *         { "source": { "type": "active task port", "port": "input", "role": "setup" },
  *           "filter": [ ["smtk::markup::Resource", null] ],
  *           "event": "deactivated" }
  *       ],
  *       "show": [
  *         { "source": { "type": "active task port", "port": "input" }, "event": "activated",
  *           "filter": [ ["smtk::markup::Resource", null] ] },
  *         { "source": { "type": "active task port", "port": "output" },
  *           "filter": [ ["*", null], ["*", "*"] ], "event": "deactivated" }
  *       ]
  *     }
  *   }
  * }
  * ```
  *
  * The example above will:
  * + switch the 3-d view to color all the project's resources by a solid color when no task is active;
  * + switch the 3-d view to color all the project's resources by whether they are associated
  *   to any attribute of type "BoundaryCondition" whenever tasks with the "example" style
  *   become active;
  * + make markup resources present on the "input" port of the active task visible (without
  *   toggling per-component visibility) when tasks with the "example" style become active.
  * + hide markup resources present on the "input" port of the active task (without toggling
  *   per-component visibility) when a task with the "example" style is deactivated; and
  * + show both resources and components (toggling as needed) on the active task's "output" port
  *   when any task with the "example" style is deactivated.
  */
class SMTKPQPROJECTEXT_EXPORT pqSMTKTaskResourceVisibility : public QObject
{
  Q_OBJECT
  using Superclass = QObject;

public:
  static pqSMTKTaskResourceVisibility* instance(QObject* parent = nullptr);
  ~pqSMTKTaskResourceVisibility() override;

protected Q_SLOTS:
  /**\brief Track projects, react to the active task.
    *
    * These methods are used to add observers to each project loaded on each server
    * so that changes to the active task of any can affect the attribute displayed
    * in this panel.
    */
  virtual void observeProjectsOnServer(pqSMTKWrapper* mgr, pqServer* server);
  virtual void unobserveProjectsOnServer(pqSMTKWrapper* mgr, pqServer* server);
  virtual void handleProjectEvent(const smtk::project::Project&, smtk::project::EventType);
  virtual void handleTaskEvent(smtk::task::Task* prevTask, smtk::task::Task* nextTask);

protected: // NOLINT(readability-redundant-access-specifiers)
  pqSMTKTaskResourceVisibility(QObject* parent = nullptr);

  void processTaskEvent(smtk::task::Task* task, smtk::string::Token event);
  void applyColorBy(const nlohmann::json& spec, smtk::task::Task* task, smtk::string::Token event);
  void applyShowObjects(
    const nlohmann::json& specArray,
    bool show,
    smtk::task::Task* task,
    smtk::string::Token event);

  /// Return a list of representations that are relevant to \a spec.
  ///
  /// These representations will correspond to a resources that match
  /// a directive above because (a) they are part of the project owning the
  /// task manager or (b) they are on input ports of the task referenced by
  /// the \a spec.
  std::map<pqRepresentation*, std::unordered_set<smtk::resource::PersistentObject*>>
  relevantRepresentations(const nlohmann::json& spec);

  /// Is the given resource relevant to the \a spec?
  bool isResourceRelevant(
    const std::shared_ptr<smtk::resource::Resource>& resource,
    const nlohmann::json& filter);

  std::map<smtk::project::ManagerPtr, smtk::project::Observers::Key> m_projectManagerObservers;
  smtk::task::Task* m_currentTask{ nullptr };
  smtk::task::Manager* m_currentTaskManager{ nullptr };
  smtk::task::Active::Observers::Key m_activeTaskObserver;
  smtk::task::Task::Observers::Key m_currentTaskObserver;

private:
  Q_DISABLE_COPY(pqSMTKTaskResourceVisibility);
};

#endif // smtk_extension_paraview_appcomponents_pqSMTKTaskResourceVisibility_h
