//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_pqSMTKAttributePanel_h
#define smtk_extension_paraview_appcomponents_pqSMTKAttributePanel_h

#include "smtk/extension/paraview/appcomponents/smtkPQComponentsExtModule.h"

#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/project/Observer.h" // for EventType
#include "smtk/resource/Observer.h"
#include "smtk/task/Task.h"

#include "smtk/PublicPointerDefs.h"

#include "pqPropertyLinks.h"

#include "smtk/extension/paraview/appcomponents/pqQtKeywordWrapping.h"

#include <QWidget>

class pqServer;
class pqPipelineSource;
class pqSMTKWrapper;

/**\brief A panel that displays a single SMTK resource for editing by the user.
  *
  * Because attribute resources currently ship with a top-level view (implied
  * if none exists), we always display that view when asked to show a resource.
  * This may change in the future.
  *
  * This panel will create a new SMTK attribute UI manager each time the
  * resource to be displayed is switched for a different resource.
  */
class SMTKPQCOMPONENTSEXT_EXPORT pqSMTKAttributePanel : public QWidget
{
  Q_OBJECT
  typedef QWidget Superclass;

public:
  pqSMTKAttributePanel(QWidget* parent = nullptr);
  ~pqSMTKAttributePanel() override;

  smtk::extension::qtUIManager* attributeUIManager() const { return m_attrUIMgr; }

Q_SIGNALS:
  void titleChanged(QString title);

public Q_SLOTS:
  /**\brief Populate the attribute panel with data from \a psrc
    *
    * If \a psrc is an attribute source, then call displayResource()
    * on the resource.
    */
  virtual bool displayPipelineSource(pqPipelineSource* psrc);
  /**\brief Populate the attribute panel with data from \a rsrc.
    *
    * If \a view is not specified, then \a rsrc's top-level view is used and the
    * \a advancedlevel is ignored.
    *  Note that displayView is used to render the appropriate view in the panel
    */
  virtual bool displayResource(
    const smtk::attribute::ResourcePtr& rsrc,
    smtk::view::ConfigurationPtr view = nullptr,
    int advancedlevel = 0);
  /**\brief Populate the attribute panel with data from \a rsrc.
    *
    * If \a view is not specified, then \a rsrc's top-level view is used and the
    * \a advancedlevel is ignored.
    *  Note that displayView is used to render the appropriate view in the panel
    * This variant does more than displayResource() alone;
    * it will obtain the wrapper associated with the resource's manager
    * and use it for selection as displayPipelineSource() does before
    * calling the plain displayResource() variant.
    */
  virtual bool displayResourceOnServer(
    const smtk::attribute::ResourcePtr& rsrc,
    smtk::view::ConfigurationPtr view = nullptr,
    int advancedlevel = 0);
  /**\brief Populate the attribute panel with the given view.
    *
    * Note that the \a view should describe an attribute resource.
    * It is possible for views to describe other things, such as the
    * configuration of a descriptive-phrase tree, but those will not
    * be accepted by this method.
    */
  virtual bool displayView(smtk::view::ConfigurationPtr view);
  /**\brief Update the attribute panel when the ParaView pipeline changes.
    *
    * The attribute resource associated with the active
    * ParaView pipeline source may have changed (i.e., the filename changed
    * and so the old resource is dropped and a new one constructed) or been
    * updated (i.e., the resource location has not changed, but the resource
    * contents have changed). Re-render.
    */
  virtual bool updatePipeline();
  /**\brief Clear panel widgets, unobserve displayed resource,
    *       and set the attribute resource pointer to null.
    */
  virtual void resetPanel(smtk::resource::ManagerPtr rsrcMgr);

  /// Raise the panel in the UI (first showing it if not shown).
  void focusPanel();

protected Q_SLOTS:
  /**\brief Called when vtkSMTKSettings is modified, indicating user preferences have changed.
    *
    * The attribute panel listens for changes to the highlight-on-hover
    * preference as well as default-value and error-value colors.
    * This slot notifies the qtUIManager when these have changed
    * and it in turn emits a signal that views or items can listen to
    * for updates.
    */
  virtual void updateSettings();

  /**\brief Connect/disconnect signals+slots controlling synchronization of
   *        ParaView pipeline sources and the attribute panel.
   *
   * This slot is attached to pqSMTKBehavior::postProcessingModeChanged()
   * so that the connections only exist during post-processing (when the
   * pipeline browser is visible).
   */
  virtual void displayActivePipelineSource(bool doDisplay);

  /**\brief Track projects, react to the active task.
    *
    * These methods are used to add observers to each project loaded on each server
    * so that changes to the active task of any can affect the attribute displayed
    * in this panel.
    */
  virtual void observeProjectsOnServer(pqSMTKWrapper* mgr, pqServer* server);
  virtual void unobserveProjectsOnServer(pqSMTKWrapper* mgr, pqServer* server);
  virtual void handleProjectEvent(const smtk::project::Project&, smtk::project::EventType);

protected:
  virtual bool updateManagers(const std::shared_ptr<smtk::common::Managers>& managers);
  virtual bool displayResourceInternal(
    const smtk::attribute::ResourcePtr& rsrc,
    smtk::view::ConfigurationPtr view = nullptr,
    int advancedlevel = 0);
  virtual void updateTitle(const smtk::view::ConfigurationPtr& view = nullptr);
  smtk::extension::qtUIManager* m_attrUIMgr{ nullptr };
  std::weak_ptr<smtk::resource::Resource> m_rsrc;
  smtk::view::SelectionPtr m_seln;
  smtk::view::ManagerPtr m_viewManager;
  smtk::operation::ManagerPtr m_opManager;
  smtk::resource::Observers::Key m_observer;
  pqPropertyLinks m_propertyLinks;

  std::map<smtk::project::ManagerPtr, smtk::project::Observers::Key> m_projectManagerObservers;
  smtk::task::Active::Observers::Key m_activeObserverKey;
  smtk::task::Task::Observers::Key m_currentTaskObserverKey;
  smtk::task::Task* m_currentTask{ nullptr };
};

#endif // smtk_extension_paraview_appcomponents_pqSMTKAttributePanel_h
