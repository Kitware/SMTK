//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_pqSMTKOperationPanel_h
#define smtk_extension_paraview_appcomponents_pqSMTKOperationPanel_h
#include "smtk/extension/paraview/appcomponents/smtkPQComponentsExtModule.h"
#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/operation/Operation.h" // for Index

#include "smtk/resource/Observer.h"

#include "smtk/PublicPointerDefs.h"

// VTK's wrapper parser does not properly handle Qt macros on macos.
#if defined(__VTK_WRAP__) && !defined(Q_SLOTS)
#define Q_DISABLE_COPY(x)
#define Q_SLOTS
#define Q_SIGNALS protected
#define Q_OBJECT
#endif

#include <QDockWidget>

class pqServer;
class pqPipelineSource;

class pqSMTKWrapper;

class QListWidgetItem;

/**\brief A panel that displays operation parameters for entry by the user.
  *
  * This panel is composed of vertical segments:
  *
  * + a list of available operations given the current selection;
  * + a panel showing operation parameters if the user indicates
  *   he wants to edit default parameters or there are parameters
  *   with no default on a requested operation;
  * + help for the operation parameters being displayed.
  *
  * Segments may be hidden depending on the context.
  *
  * This panel will create a new SMTK attribute UI manager each time the
  * operation to be displayed is switched for a different resource.
  */
class SMTKPQCOMPONENTSEXT_EXPORT pqSMTKOperationPanel : public QDockWidget
{
  Q_OBJECT
  typedef QDockWidget Superclass;

public:
  pqSMTKOperationPanel(QWidget* parent = nullptr);
  ~pqSMTKOperationPanel() override;

  smtk::extension::qtUIManager* attributeUIManager() const { return m_attrUIMgr; }

  smtk::view::AvailableOperationsPtr availableOperations() const { return m_availableOperations; }

public Q_SLOTS:
  /// Called when a new client-server connection is added.
  virtual void observeWrapper(pqSMTKWrapper*, pqServer*);

  /// Called when a client-server connection is removed.
  virtual void unobserveWrapper(pqSMTKWrapper*, pqServer*);

  /**\brief Populate the operation editor panel with the given operation \a index.
    *
    * If the operation editor is hidden and this method returns true,
    * it will be shown.
    */
  virtual bool editOperation(smtk::operation::Operation::Index index);

  /**\brief Populate the operation panel with the given \a operation.
    *
    * If the operation editor is hidden and this method returns true,
    * it will be shown.
    */
  virtual bool editOperation(smtk::operation::OperationPtr operation);

  /// Queue the (potentially asynchronous) operation whose parameters are shown to be run.
  virtual void runOperation();

  /// Queue the (potentially asynchronous) operation to be run immediately with default parameters.
  virtual void runOperation(smtk::operation::OperationPtr operation);

  /**\brief Abort editing operation parameters and reset the operation panel.
    *
    * This will not cancel any running operations.
    */
  virtual void cancelEditing();

protected Q_SLOTS:
  virtual void toggleFilterBySelection(bool);
  virtual void operationListClicked(QListWidgetItem* item);
  virtual void operationListDoubleClicked(QListWidgetItem* item);
  virtual void operationListActivated(QListWidgetItem* item);
  virtual void operationListCurrentItemChanged(QListWidgetItem* item, QListWidgetItem* prev);

protected:
  /// Show documentation for the given operation.
  virtual void displayDocumentation(const smtk::operation::Operation::Index& index);

  class Internal;
  Internal* m_p{ nullptr };
  pqSMTKWrapper* m_wrapper{
    nullptr
  }; // TODO: Remove the need for me. This ties us to a single pqServer.
  smtk::operation::OperationPtr m_editing;
  smtk::extension::qtUIManager* m_attrUIMgr{ nullptr };
  std::weak_ptr<smtk::resource::Resource> m_rsrc;
  smtk::view::AvailableOperationsPtr m_availableOperations;
  smtk::resource::Observers::Key m_observer;
};

#endif // smtk_extension_paraview_appcomponents_pqSMTKOperationPanel_h
