//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponenets_pqSMTKWrapper_h
#define smtk_extension_paraview_appcomponenets_pqSMTKWrapper_h

#include "smtk/extension/paraview/appcomponents/Exports.h"

#include "smtk/operation/NewOp.h"
#include "smtk/operation/Observer.h"

#include "smtk/PublicPointerDefs.h"

#include "pqProxy.h"

#include <functional>
#include <set>

class pqOutputPort;
class pqSMTKResource;
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
  pqSMTKWrapper(const QString& regGroup, const QString& regName, vtkSMProxy* proxy,
    pqServer* server, QObject* parent = nullptr);
  ~pqSMTKWrapper() override;

  /// Return the proxy for the resource manager cast to its proper type.
  vtkSMSMTKWrapperProxy* smtkProxy() const;
  /// Return the client-side resource manager which mirrors the server version.
  smtk::resource::ManagerPtr smtkResourceManager() const;
  /// Return the client-side operation manager which mirrors the server version.
  smtk::operation::ManagerPtr smtkOperationManager() const;
  /// Return the client-side selection which mirrors the server version.
  smtk::view::SelectionPtr smtkSelection() const;

  /// Return the pqSMTKResource which owns the given smtk::resource::ResourcePtr.
  pqSMTKResource* getPVResource(smtk::resource::ResourcePtr rsrc) const;

  /**\brief Invoke the \a visitor function on each pqSMTKResource attached to this manager.
    *
    * If the \a visitor returns true, then iteration will terminate early.
    */
  void visitResources(std::function<bool(pqSMTKResource*)> visitor) const;

public slots:
  /// Called by pqSMTKBehavior to add resources as they are created.
  virtual void addResource(pqSMTKResource* rsrc);
  /// Called by pqSMTKBehavior to remove resources as they are destroyed.
  virtual void removeResource(pqSMTKResource* rsrc);

signals:
  /// Emitted when a new resource is added to the manager.
  void resourceAdded(pqSMTKResource* rsrc);
  /// Emitted when a resource is removed from the manager.
  void resourceRemoved(pqSMTKResource* rsrc);
  /**\brief Signal that an operator \a op has been created, is about to run,
    *       or has run with the included \a result.
    */
  void operationEvent(smtk::operation::NewOp::Ptr op, smtk::operation::EventType event,
    smtk::operation::NewOp::Result result);

protected slots:
  virtual void paraviewSelectionChanged(pqOutputPort* port);

protected:
  std::set<QPointer<pqSMTKResource> > m_resources;

private:
  Q_DISABLE_COPY(pqSMTKWrapper);
};

#endif
