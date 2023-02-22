//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_project_pqSMTKDisplayProjectOnLoadBehavior_h
#define smtk_extension_paraview_project_pqSMTKDisplayProjectOnLoadBehavior_h

#include "smtk/extension/paraview/project/smtkPQProjectExtModule.h"

#include "smtk/PublicPointerDefs.h"

#include "smtk/project/Observer.h" // for EventType

#include "smtk/extension/paraview/appcomponents/pqQtKeywordWrapping.h"

#include <QObject>

#include <string>

class vtkSMSMTKWrapperProxy;

class pqServer;

namespace smtk
{
namespace task
{
class Manager;
}
} // namespace smtk

/**\brief Make the SMTK task panel display a project when one is loaded.
  *
  * This is accomplished by adding an observer to the application's project manager.
  * When a project is loaded, the task panel is made visible and raised to the top
  * (if in a tabbed dock).
  */
class SMTKPQPROJECTEXT_EXPORT pqSMTKDisplayProjectOnLoadBehavior : public QObject
{
  Q_OBJECT
  using Superclass = QObject;

public:
  pqSMTKDisplayProjectOnLoadBehavior(QObject* parent = nullptr);
  ~pqSMTKDisplayProjectOnLoadBehavior() override;

  /// This behavior is a singleton.
  static pqSMTKDisplayProjectOnLoadBehavior* instance(QObject* parent = nullptr);

protected Q_SLOTS:
  //@{
  /// Observe server connections/disconnections so we can monitor each server's
  /// projects for updates and, on project load/creation, force the task panel
  /// to the top.
  virtual void observeProjectsOnServer(vtkSMSMTKWrapperProxy* mgr, pqServer* server);
  virtual void unobserveProjectsOnServer(vtkSMSMTKWrapperProxy* mgr, pqServer* server);
  virtual void handleProjectEvent(const smtk::project::Project&, smtk::project::EventType);
  virtual void focusTaskPanel(smtk::task::Manager* taskManager);
  //@}

protected:
  std::map<smtk::project::ManagerPtr, smtk::project::Observers::Key> m_projectManagerObservers;

private:
  Q_DISABLE_COPY(pqSMTKDisplayProjectOnLoadBehavior);
};

#endif
