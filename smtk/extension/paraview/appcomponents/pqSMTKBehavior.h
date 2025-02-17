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

#include "smtk/view/Utility.h" // For smtk::view::{ResourceMap,SharedResourceMap}

#include <QObject>

#include <functional>
#include <unordered_map>
#include <unordered_set>

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

  /**\brief Update the map from servers to SMTK wrappers.
    *
    * In the ModelBuilder application, if the Python Shell panel is
    * started after the application has started, the builtin client
    * will not have an entry in `m_p->Remotes` yet. This function will
    * ensure the active server has an entry so that Python methods
    * on the behavior (such as activeWrapperCommonManagers()) do not
    * return a null object.
    */
  void updateWrapperMap();

  /**\brief Show (or hide) the given \a objects.
    *
    * If \a show is true, every object in \a objects is made visible
    * and a render is requested; otherwise, every object in \a objects
    * is hidden and a render is requested.
    *
    * If an entry in \a objects is a resource, any pqRepresentation
    * corresponding to the pqPipeline is shown/hidden.
    * If an entry in \a objects is a component, matching pqRepresentation
    * objects for the component's resource are located. The representation's
    * visibility is toggled id \a show is true; otherwise it is untouched.
    * Then the per-block visibility of the component is toggled to match \a show.
    *
    * Thus, hiding a component will never modify the resource's visibility
    * while showing a component may toggle the resources's visibility (as well
    * as the block matching the component).
    *
    * This function returns true if a change was made (i.e., a render() has
    * been requested).
    */
  bool showObjects(
    bool show,
    const std::set<smtk::resource::PersistentObject*>& objects,
    pqView* view = nullptr);

  /**\brief This version of showObjects accepts a \a visibilityMap.
    *
    * Because the \a visibilityMap segregates objects into resources and their components,
    * it is efficient; the resource's visibility is only handled once and all of the block
    * visibilities that require changing are handled at the same time.
    */
  bool showObjects(bool show, const smtk::view::ResourceMap& visibilityMap, pqView* view = nullptr);
  bool showObjects(
    bool show,
    const smtk::view::SharedResourceMap& visibilityMap,
    pqView* view = nullptr);

  /**\brief Register operations from the named module after the event loop starts.
    *
    * ParaView loads plugins (including python plugins) before a
    * server connection is established. This method queues a function
    * to run after the event loop starts to register Python operations
    * to the active server's operation manager.
    */
  static void importPythonOperationsForModule(
    const std::string& moduleName,
    const std::string& operationName);

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
  /// Connected in pqSMTKAppComponentsAutoStart::startup()
  void pipelineSourceCreated(
    smtk::resource::ResourcePtr smtkResource,
    pqSMTKResource* pipelineSource);
  /// Connected in pqSMTKAppComponentsAutoStart::startup()
  void aboutToDestroyPipelineSource(
    smtk::resource::ResourcePtr smtkResource,
    pqSMTKResource* pipelineSource);

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
