//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_job_pqJobRunnerView_h
#define smtk_extension_paraview_job_pqJobRunnerView_h

#include "smtk/extension/paraview/job/smtkPVJobExtModule.h"

#include "smtk/extension/qt/qtBaseView.h"

#include "smtk/attribute/Attribute.h"

#include <QPointer>

#include <memory>

class pqPipelineSource;
class pqDataRepresentation;

namespace smtk
{
namespace job
{
class Job;
}
} // namespace smtk

/** \brief A custom smtk view allowing users to create, schedule, and monitor a job.
  *
  * Users are provided with a button to run an operation, inspect the log file(s)
  * from the running/prior job, and schedule/cancel/retry a job.
  *
  * This view interacts with a JobAgent active when the view is first displayed.
  * The operation is expected to (1) be provided (constructed and configured) by
  * the agent named in the view configuration and (2) create a job, placing it
  * in the "created" item of the its result.
  *
  */
class SMTKPVJOBEXT_EXPORT pqJobRunnerView : public smtk::extension::qtBaseView
{
  Q_OBJECT
public:
  smtkSuperclassMacro(smtk::extension::qtBaseView);
  smtkTypenameMacro(pqJobRunnerView);

  static qtBaseView* createViewWidget(const smtk::view::Information& info);
  pqJobRunnerView(const smtk::view::Information& info);
  ~pqJobRunnerView() override;

  bool isEmpty() const override { return false; }

  /// Return a pointer to the most recently-run job.
  smtk::job::Job* currentJob() const;

public Q_SLOTS:
  /** \brief Update controls based on current project state.
    *
    * This is called each time the view is displayed, which is
    * typically when its parent tab is selected.
    */
  void updateUI() override;

protected Q_SLOTS:
  /** \brief Fetches an operation from it's task's agent, adds a custom handler,
    *        and launches the operation.
    *
    * The custom handler notifies the agent that the job has been created.
    * On success of the operation, SMTK's job manager will launch an
    * AddJobToQueue operation to add the job to its queue. That in turn
    * may launch ScheduleJob if the job is marked to be auto-scheduled.
    * The view monitors all of these things occurring and provides users
    * with feedback on the job state.
    */
  void onRunClicked();

  /// Update the timestamp label and button for displaying the log.
  ///
  /// If no log file exists, the button will be hidden and the label will
  /// indicate no file exists.
  void updateJobControls();

protected:
  /// Create the view's top-level widget.
  void createWidget() override;

private:
  class Internal;
  std::unique_ptr<Internal> m_p;
};

#endif // smtk_extension_paraview_job_pqJobRunnerView_h
