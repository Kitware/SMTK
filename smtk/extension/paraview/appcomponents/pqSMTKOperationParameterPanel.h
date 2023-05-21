//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_pqSMTKOperationParameterPanel_h
#define smtk_extension_paraview_appcomponents_pqSMTKOperationParameterPanel_h
#include "smtk/extension/paraview/appcomponents/smtkPQComponentsExtModule.h"
#include "smtk/extension/qt/qtOperationPalette.h"
#include "smtk/extension/qt/qtOperationView.h"
#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/project/Observer.h" // for EventType

#include "smtk/task/Task.h" // for Task::Observers::Key

#include "smtk/operation/Operation.h" // for Index

#include "smtk/resource/Observer.h"

#include "smtk/PublicPointerDefs.h"

#include "smtk/extension/paraview/appcomponents/pqQtKeywordWrapping.h"

#include <QPointer>
#include <QWidget>

#include <map>
#include <set>

class QTabWidget;

class pqModalShortcut;
class pqPipelineSource;
class pqServer;

class pqSMTKWrapper;

class QListWidgetItem;
class QVBoxLayout;

/**\brief A panel that displays available operations in a "toolbox".
  *
  * The panel emits signals when users request an operation be
  * (a) immediately run or (b) run after editing parameters.
  */
class SMTKPQCOMPONENTSEXT_EXPORT pqSMTKOperationParameterPanel : public QWidget
{
  Q_OBJECT
  typedef QWidget Superclass;

public:
  pqSMTKOperationParameterPanel(QWidget* parent = nullptr);
  ~pqSMTKOperationParameterPanel() override;

Q_SIGNALS:
  void titleChanged(QString title);

public Q_SLOTS:
  /// Called when a new client-server connection is added.
  virtual void observeWrapper(pqSMTKWrapper*, pqServer*);

  /// Called when a client-server connection is removed.
  virtual void unobserveWrapper(pqSMTKWrapper*, pqServer*);

  /**\brief Called in response to signals from the pqSMTKOperationToolboxPanel.
    *
    * This method will create and launch an operation of the given type
    * immediately (using default parameter values).
    * If the operation cannot be run with defaults (i.e., ableToOperate()
    * returns false), then editOperationParametrs is invoked.
    */
  virtual void runOperationWithDefaults(smtk::operation::Operation::Index);

  /**\brief Queue the (potentially asynchronous) operation to be run immediately as specified.
    *
    */
  void runOperationWithParameters(const std::shared_ptr<smtk::operation::Operation>& operation);

  /**\brief Called in response to signals from the pqSMTKOperationToolboxPanel.
    *
    * This method will raise the panel and create or switch to a tab
    * for the given operation.
    */
  virtual void editOperationParameters(smtk::operation::Operation::Index);

  /**\brief Called in response to signals from pqSMTKOperationToolboxPanel.
    *
    * This method will raise the panel and create or switch to a tab
    * for the given operation.
    * Unlike the variant that takes an operation index, this variant
    * accepts an operation which may have non-default parameters.
    */
  void editExistingOperationParameters(
    const std::shared_ptr<smtk::operation::Operation>& operation,
    bool associateSelection = true,
    bool isTabClosable = true,
    bool showApply = true);

  /// Called when users close an operation-parameter's tab.
  virtual void cancelEditing(int tabIndex);

  /// Find the tab for the corresponding \a operation and close it.
  ///
  /// If no such tab existed or if the tab could not be closed,
  /// this will return false.
  ///
  /// If \a forceClose is true, then any mark that made the tab
  /// un-closable by user action is unset, forcing the method
  /// to succeed as long as a tab exists for the operation.
  bool closeTabForOperation(
    const std::shared_ptr<smtk::operation::Operation>& operation,
    bool forceClose = false);

  /// Called when users click "Done" in a qtOperationView (uses signal sender to find tab).
  virtual void cancelTabFromSender();

  /// Raise the panel in the UI (first showing it if not shown).
  void focusPanel();

protected Q_SLOTS:
  /**\brief Track projects, react to the active task.
    *
    * These methods are used to add observers to each project loaded on each server
    * so that changes to the active task of any can affect the operation displayed
    * in this panel.
    */
  virtual void observeProjectsOnServer();
  virtual void unobserveProjectsOnServer();
  virtual void handleProjectEvent(const smtk::project::Project&, smtk::project::EventType);

Q_SIGNALS:
  /// Queue the (potentially asynchronous) operation to be run immediately with default parameters.
  void runOperation(smtk::operation::Operation::Index index);

protected:
  struct TabData;

  void observeToolboxPanels();

  /// Return the tab-view metadata for the given operation (if any).
  TabData* tabDataForOperation(smtk::operation::Operation& op);

  /// Called when the panel is displaying a SubmitOperation task and the task changes state.
  void activeTaskStateChange(
    smtk::task::Task& task,
    smtk::task::State priorState,
    smtk::task::State currentState);

  struct TabData
  {
    std::shared_ptr<smtk::operation::Operation> m_operation;
    QPointer<QWidget> m_tab;
    QPointer<smtk::extension::qtUIManager> m_uiMgr;
    QPointer<smtk::extension::qtBaseView> m_view;
    bool m_closable{ true }; // Can this tab be closed?
  };
  QPointer<QTabWidget> m_tabs;
  QPointer<QVBoxLayout> m_layout;
  pqSMTKWrapper* m_wrapper{ nullptr }; // NB: This ties us to a single pqServer (the active one).
  std::multimap<smtk::operation::Operation::Index, TabData> m_views;
  int m_selectionValue{ 1 }; // What int/bits in the selection map should be tied to associations?
  bool m_selectionExactMatch{ false }; // Exactly match m_selectionValue (integer) or not (bits)?

  std::map<smtk::project::ManagerPtr, smtk::project::Observers::Key> m_projectManagerObservers;
  smtk::task::Active::Observers::Key m_activeObserverKey;
  smtk::task::Task::Observers::Key m_taskObserver;
};

#endif // smtk_extension_paraview_appcomponents_pqSMTKOperationParameterPanel_h
