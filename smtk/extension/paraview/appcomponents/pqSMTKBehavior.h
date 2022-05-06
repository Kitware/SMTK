//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_pqSMTKBehavior_h
#define smtk_extension_paraview_appcomponents_pqSMTKBehavior_h

#include "smtk/extension/paraview/appcomponents/smtkPQComponentsExtModule.h"

#include "smtk/PublicPointerDefs.h"

#include "smtk/extension/paraview/appcomponents/pqQtKeywordWrapping.h"

#include <QObject>

#include <functional>

class pqOutputPort;
class pqPipelineSource;
class pqProxy;
class pqSelectionManager;
class pqServer;
class pqSMTKResource;
class pqSMTKWrapper;
class pqView;
class vtkSMSMTKWrapperProxy;
class vtkSMProxy;

class pqSMTKAppComponentsAutoStart;

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
class SMTKPQCOMPONENTSEXT_EXPORT pqSMTKBehavior : public QObject
{
  Q_OBJECT
  using Superclass = QObject;

  // Allow this plugin's AutoStart to register an active server, if there is one.
  friend class pqSMTKAppComponentsAutoStart;

public:
  static pqSMTKBehavior* instance(QObject* parent = nullptr);
  ~pqSMTKBehavior() override;

  /**\brief Return the server-side container of counterparts for the managers above.
    *
    * If \a server is null, a random server's proxy is chosen.
    * Since most apps connect to a single server, this is usually well-defined.
    */
  ///@{
  ///
  vtkSMSMTKWrapperProxy* wrapperProxy(pqServer* server = nullptr) const;
  ///
  pqSMTKWrapper* resourceManagerForServer(pqServer* server = nullptr) const;
  ///@}

  virtual void addPQProxy(pqSMTKWrapper* rsrcMgr);

  /// Return the pqSMTKWrapper for a given smtk::resource::ManagerPtr.
  pqSMTKWrapper* getPVResourceManager(smtk::resource::ManagerPtr rsrcMgr);

  /// Return the pqSMTKResource for a given smtk::resource::ResourcePtr.
  ///
  /// Note that this is an O(N) operation (linear in the number of
  /// resources loaded across all servers).
  QPointer<pqSMTKResource> getPVResource(const smtk::resource::ResourcePtr& rsrc) const;

  /// Return the vtkSMProxy for a given smtk::resource::ResourcePtr.
  vtkSMProxy* getPVResourceProxy(const smtk::resource::ResourcePtr& rsrc) const;

  /// process non-user QT events
  static void processEvents();

  /**\brief Call a visitor function \a fn on each existing resource manager/server pair.
    *
    * This method has the same signature as the addedManagerOnServer signal, so you
    * can pass a simple lambda that invokes your class's slot to the signal.
    * This way, you can attach signals to resource managers already in existence
    * in addition to those created after your class's initialization.
    *
    * If the function returns true, then iteration should terminate.
    */
  virtual void visitResourceManagersOnServers(
    const std::function<bool(pqSMTKWrapper*, pqServer*)>& fn) const;

  /**
   * Create a pqDataRepresentation and set its default visibility value.
   */
  bool createRepresentation(pqSMTKResource* pvr, pqView* view);

  /**\brief Return the SMTK wrapper serving the client process, or,
   *        failing that, the active server's wrapper (or null if
   *        none can be located).
   */
  pqSMTKWrapper* builtinOrActiveWrapper() const;

  /**\brief Return whether SMTK is in "post-processing" mode or not.
   */
  bool postProcessingMode() const { return m_postProcessingMode; }

public Q_SLOTS:
  /// Set whether post-processing mode is enabled (true) or disabled (false; default).
  ///
  /// The return value indicates whether the value changed.
  virtual bool setPostProcessingMode(bool inPost);

Q_SIGNALS:
  /// Called from within addManagerOnServer (in response to server becoming ready)
  void addedManagerOnServer(vtkSMSMTKWrapperProxy* mgr, pqServer* server);
  void addedManagerOnServer(pqSMTKWrapper* mgr, pqServer* server);
  /// Called from within removeManagerFromServer (in response to server disconnect prep)
  void removingManagerFromServer(vtkSMSMTKWrapperProxy* mgr, pqServer* server);
  void removingManagerFromServer(pqSMTKWrapper* mgr, pqServer* server);
  /// Called from within setPostProcessingMode.
  void postProcessingModeChanged(bool isPostProcessing);

protected:
  pqSMTKBehavior(QObject* parent = nullptr);

  void setupSelectionManager();

  /**
   * Make a representation visible through a ControllerWithRendering instance.
   */
  void setDefaultRepresentationVisibility(pqOutputPort* pqPort, pqView* view);

  class Internal;
  Internal* m_p;
  bool m_postProcessingMode{ false };

protected Q_SLOTS:
  /// Called whenever a PV server becomes ready.
  virtual void addManagerOnServer(pqServer*);
  /// Called whenever a PV server is about to be disconnected.
  virtual void removeManagerFromServer(pqServer*);
  /// Track when SMTK proxies are added (not all \a pxy may cast to SMTK objects but some may)
  virtual void handleNewSMTKProxies(pqProxy* pxy);
  /// Track when resources are removed (not all \a pxy may cast to SMTK objects but some may)
  virtual void handleOldSMTKProxies(pqPipelineSource* pxy);
  /// Track when SMTK proxy resources are changed.
  virtual void updateResourceProxyMap(const std::shared_ptr<smtk::resource::Resource>& resource);

private:
  Q_DISABLE_COPY(pqSMTKBehavior);
};

#endif // smtk_extension_paraview_appcomponents_pqSMTKBehavior_h
