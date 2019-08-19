//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/Exports.h"
#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/operation/Operation.h" // for Index

#include "smtk/resource/Observer.h"

#include "smtk/PublicPointerDefs.h"

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

public slots:
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

protected slots:
  virtual void toggleFilterBySelection(bool);
  virtual void operationListClicked(QListWidgetItem* item);
  virtual void operationListDoubleClicked(QListWidgetItem* item);
  virtual void operationListActivated(QListWidgetItem* item);
  virtual void operationListCurrentItemChanged(QListWidgetItem* item, QListWidgetItem* prev);

protected:
  /// Show documentation for the given operation.
  virtual void displayDocumentation(const smtk::operation::Operation::Index& index);

  class Internal;
  Internal* m_p;
  pqSMTKWrapper* m_wrapper; // TODO: Remove the need for me. This ties us to a single pqServer.
  smtk::operation::OperationPtr m_editing;
  smtk::extension::qtUIManager* m_attrUIMgr;
  std::weak_ptr<smtk::resource::Resource> m_rsrc;
  smtk::view::AvailableOperationsPtr m_availableOperations;
  smtk::resource::Observers::Key m_observer;
};
