//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_pqSMTKResourceManagerBehavior_h
#define smtk_extension_paraview_appcomponents_pqSMTKResourceManagerBehavior_h

#include "smtk/extension/paraview/appcomponents/Exports.h"

#include "smtk/PublicPointerDefs.h"

#include <QObject>

class pqOutputPort;
class pqSelectionManager;
class pqServer;
class vtkSMSMTKResourceManagerProxy;

/** \brief Create and synchronize smtk manager instances on the client and server.
  *
  * This instance will watch for server connection/disconnection events.
  * When a new server is connected, it will have the server-manager instantiate
  * an smtk::resource::Manager there.
  * When a server is disconnected, it will remove information from the client about
  * that server's resources.
  *
  * At the moment, all this class does is create a resource manager and expose it.
  * It should eventually deal with synchronization issues as stated above, but
  * SMTK's resource manager doesn't deal with remote resources yet.
  */
class SMTKPQCOMPONENTSEXT_EXPORT pqSMTKResourceManagerBehavior : public QObject
{
  Q_OBJECT
  using Superclass = QObject;

public:
  static pqSMTKResourceManagerBehavior* instance(QObject* parent = nullptr);
  ~pqSMTKResourceManagerBehavior() override;

  /**\brief Return the server-side container of counterparts for the managers above.
    *
    * If \a server is null, a random server's proxy is chosen.
    * Since most apps connect to a single server, this is usually well-defined.
    */
  vtkSMSMTKResourceManagerProxy* wrapperProxy(pqServer* server = nullptr);

signals:
  /// Called from within addManagerOnServer (in response to server becoming ready)
  void addedManagerOnServer(vtkSMSMTKResourceManagerProxy* mgr, pqServer* server);
  /// Called from within removeManagerFromServer (in response to server disconnect prep)
  void removingManagerFromServer(vtkSMSMTKResourceManagerProxy* mgr, pqServer* server);

protected:
  pqSMTKResourceManagerBehavior(QObject* parent = nullptr);

  void setupSelectionManager();

  class Internal;
  Internal* m_p;

protected slots:
  /// Called whenever a PV server becomes ready.
  virtual void addManagerOnServer(pqServer*);
  /// Called whenever a PV server is about to be disconnected.
  virtual void removeManagerFromServer(pqServer*);

private:
  Q_DISABLE_COPY(pqSMTKResourceManagerBehavior);
};

#endif // smtk_extension_paraview_appcomponents_pqSMTKResourceManagerBehavior_h
