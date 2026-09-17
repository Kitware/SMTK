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

/**\brief Let the active task control the visibility of resources/components
  *       and the postprocessing mode (for cmb-based applications).
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
  * The "filter" must be an array holding:
  *
  * + dictionaries with a "resource" and "component" key or
  * + arrays holding two strings (a resource and component filter, in that order).
  *
  * When a task is deactivated and no new task is activated at the same time,
  * (1) if the task-path is empty (i.e., the top-level tasks are showing), then the default
  *     styles are applied; or
  * (2) if the task-path is non-empty, the right-most task's style is applied.
  *
  * If you wish to provide a project default (when no tasks are active but a task-manager
  * is present), include a style for the tag named "default".
  *
  * The boolean "paraview-mode" directive is applied on activation. Older
  * "postprocessing": { "mode": true/false } dictionaries remain supported;
  * "paraview-mode" takes precedence when both forms are present.
  * Independently, a top-level "layout" directive selects a named layout on the
  * active server. It may be a name string or a dictionary with "name" and "views".
  * "views" is an array containing one view node or two child nodes. A leaf node
  * specifies a ParaView proxy "type" (e.g., "RenderView" or "XYChartView") and
  * an optional non-empty "name" for its registered name and visible frame title. A pair
  * requires "split": "vertical" (top/bottom) or "horizontal" (left/right), with
  * an optional "fraction" between 0 and 1 (default 0.5). Child nodes may themselves
  * contain "views" and a "split" to form nested layouts.
  *
  * For example, one chart above two charts is:
  * ```json
  * "layout": {
  *   "name": "PostProcessing", "split": "vertical",
  *   "views": [
  *     { "type": "XYChartView" },
  *     { "split": "horizontal", "views": [
  *       { "type": "XYChartView" }, { "type": "XYChartView" }
  *     ] }
  *   ]
  * }
  * ```
  * The tree initializes new layouts or existing unsplit, empty layouts. Populated
  * layouts retain their views and splits; explicit names are reapplied to matching
  * views at their configured locations on activation. Layouts and modes are applied
  * before 3d-view directives so those directives can affect newly created views.
  *
  * A top-level "job-results" directive can populate those views on activation.
  * It reads a completed, successful smtk::job::Job from the task's input port
  * ("port" defaults to "input", "role" defaults to "job") and opens "directory"
  * relative to Job::caseDirectory() (default "postProcessing"). The directory
  * must exist. "reader" names a ParaView source proxy with a FileName property.
  * "routes" maps output-port wildcard patterns to view names in the selected
  * layout, for example:
  * ```json
  * "job-results": {
  *   "reader": "CorpsFoamPostProcessingReader",
  *   "routes": { "flow": "Flow Results", "probes-*": "Probes" }
  * }
  * ```
  * Readers are shared with job artifact controls through pqArtifacts using the
  * results directory and the "job" tag. Results managed by this
  * directive are hidden before applying new routes, including when no successful
  * job or results directory is available. Unrelated user-created plots are retained.
  *
  * An example is:
  * ```json
  * "styles": {
  *   "default": { "3d-view": { "color-by": { "mode": "none" } }, "paraview-mode": false },
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
  *     },
  *     "paraview-mode": true,
  *     "layout": "PostProcessing"
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
  *   when any task with the "example" style is deactivated. (This way, as long as the task is
  *   active, its input port data is visible; when deactivated, its output port data is visible.)
  * + turn CMB's postprocessing mode off when no task is active and on when a task marked with
  *   the "example" style is active. If the active task has no style indicating a postprocessing
  *   mode, all parent tasks of the active task are traversed and their styles examined.
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
  void applyLayout(const nlohmann::json& spec);
  void applyJobResults(const nlohmann::json& spec, smtk::task::Task* task);
  void applyColorBy(const nlohmann::json& spec, smtk::task::Task* task, smtk::string::Token event);
  void applyShowObjects(
    const nlohmann::json& specArray,
    bool show,
    smtk::task::Task* task,
    smtk::string::Token event);

  std::map<smtk::project::ManagerPtr, smtk::project::Observers::Key> m_projectManagerObservers;
  smtk::task::Task* m_currentTask{ nullptr };
  smtk::task::Manager* m_currentTaskManager{ nullptr };
  std::map<smtk::task::Manager*, smtk::task::Active::Observers::Key> m_activeTaskObservers;
  smtk::task::Task::Observers::Key m_currentTaskObserver;

private:
  Q_DISABLE_COPY(pqSMTKTaskResourceVisibility);
};

#endif // smtk_extension_paraview_appcomponents_pqSMTKTaskResourceVisibility_h
