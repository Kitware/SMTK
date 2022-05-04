//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_pqSMTKWrapper_h
#define smtk_extension_paraview_appcomponents_pqSMTKWrapper_h

#include "smtk/extension/paraview/appcomponents/smtkPQComponentsExtModule.h"

#include "smtk/common/Managers.h"

#include "smtk/operation/Observer.h"
#include "smtk/operation/Operation.h"

#include "smtk/PublicPointerDefs.h"

#include "smtk/extension/paraview/appcomponents/pqQtKeywordWrapping.h"

#include "pqProxy.h"

#include <functional>
#include <set>

class pqOutputPort;
class pqSMTKResource;
class vtkSelection;
class vtkSMSourceProxy;
class vtkSMSMTKWrapperProxy;

/**\brief A proxy for SMTK resource managers created when connecting to a new server.
  *
  * Each time ParaView connects to a new server, the pqSMTKBehavior
  * will direct the pqObjectBuilder to create a new vtkSMTKWrapper
  * instance on each of that server's processes, plus a proxy on the client.
  * This class exists to expose signals and slots related to these objects.
  *
  * Note that ParaView itself does not allow multiple simultaneous server connections
  * but the framework does not prevent it, so custom applications built on
  * ParaView's libraries may choose to allow it.
  * Thus it is possible for multiple SMTK resource managers to coexist.
  *
  * Finally, note that pqSMTKResource instances do not have a 1-to-1 relationship
  * with smtk::resource::Resource instances; a pqSMTKResource may exist before there
  * is a valid SMTK resource and may change which SMTK resource it refers to during
  * its lifetime. (A pqSMTKResource is a pipeline source object whose filename property
  * may change while SMTK resources are tied to the contents of a particular file.)
  */
class SMTKPQCOMPONENTSEXT_EXPORT pqSMTKWrapper : public pqProxy
{
  Q_OBJECT
  using Superclass = pqProxy;

public:
  pqSMTKWrapper(
    const QString& regGroup,
    const QString& regName,
    vtkSMProxy* proxy,
    pqServer* server,
    QObject* parent = nullptr);
  ~pqSMTKWrapper() override;

  /// Return the proxy for the resource manager cast to its proper type.
  vtkSMSMTKWrapperProxy* smtkProxy() const;
  /// Return the client-side resource manager which mirrors the server version.
  smtk::resource::ManagerPtr smtkResourceManager() const;
  /// Return the client-side operation manager which mirrors the server version.
  smtk::operation::ManagerPtr smtkOperationManager() const;
  /// Return the client-side selection which mirrors the server version.
  smtk::view::SelectionPtr smtkSelection() const;
  /// Return the client-side project manager which mirrors the server version.
  smtk::project::ManagerPtr smtkProjectManager() const;
  /// Return the client-side view manager which mirrors the server version.
  smtk::view::ManagerPtr smtkViewManager() const;

  /// Return the client-side collection of managers which mirrors the server version.
  smtk::common::TypeContainer& smtkManagers() const;
  smtk::common::Managers::Ptr smtkManagersPtr() const;

  /// Return the pqSMTKResource which owns the given smtk::resource::ResourcePtr.
  pqSMTKResource* getPVResource(const smtk::resource::ResourcePtr& rsrc) const;

  /**\brief Invoke the \a visitor function on each pqSMTKResource attached to this manager.
    *
    * If the \a visitor returns true, then iteration will terminate early.
    */
  void visitResources(std::function<bool(pqSMTKResource*)> visitor) const;

public Q_SLOTS:
  /// Called by pqSMTKBehavior to add resources as they are created.
  virtual void addResource(pqSMTKResource* rsrc);
  /// Called by pqSMTKBehavior to remove resources as they are destroyed.
  virtual void removeResource(pqSMTKResource* rsrc);

protected:
  std::set<QPointer<pqSMTKResource>> m_resources;

private:
  Q_DISABLE_COPY(pqSMTKWrapper);
};

#endif
