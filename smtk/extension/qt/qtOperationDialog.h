//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtOperationDialog - A modal dialog for launching SMTK Operations

#ifndef smtk_extension_qt_qtOperationDialog_h
#define smtk_extension_qt_qtOperationDialog_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/extension/qt/Exports.h"
#include "smtk/operation/Operation.h"

#include <QDialog>
#include <QSharedPointer>

class QShowEvent;
class QWidget;
class qtOperationDialogInternals;

/**\brief Provides a model dialog for launching SMTK operations.
 *
 * The intended use is for modelbuilder plugins that use menu or similar actions to invoke
 * SMTK operations. (For example, export operations are typically run from a modal dialog.)
 * The dialog is created as a QTabWidget with 2 tabs. The first tab embeds a qtOperationView
 * for the operation, and the second tab displays the operation's "info" content. The
 * dialog replaces and hides the "Apply", "Info" and "Cancel" buttons and consumes their Qt
 * connections.
*/

namespace smtk
{
namespace extension
{

class qtUIManager;

// Modal dialog for smtk operation view.
class SMTKQTEXT_EXPORT qtOperationDialog : public QDialog
{
  Q_OBJECT

public:
  qtOperationDialog(
    smtk::operation::OperationPtr operation,
    QSharedPointer<smtk::extension::qtUIManager> uiManager,
    QWidget* parentWidget = nullptr);
  qtOperationDialog(
    smtk::operation::OperationPtr operation,
    smtk::resource::ManagerPtr resourceManager,
    smtk::view::ManagerPtr viewManager,
    QWidget* parentWidget = nullptr);

  // Use this constructor to include a scrolling area in the opertion
  // view. Only set this flag if the operation parameters are
  // lengthy and take up alot of vertical display space.
  qtOperationDialog(
    smtk::operation::OperationPtr operation,
    smtk::resource::ManagerPtr resourceManager,
    smtk::view::ManagerPtr viewManager,
    bool scrollable,
    QWidget* parentWidget = nullptr);
  virtual ~qtOperationDialog();

signals:
  void operationExecuted(const smtk::operation::Operation::Result& result);

public slots:

protected slots:
  void onOperationExecuted(const smtk::operation::Operation::Result& result);

protected:
  void buildUI(
    smtk::operation::OperationPtr op,
    QSharedPointer<smtk::extension::qtUIManager> uiMManager,
    bool scrollable = false);

  // Override showEvent() in order to fix Qt sizing issue
  void showEvent(QShowEvent* event) override;

private:
  qtOperationDialogInternals* m_internals;
}; // class
} // namespace extension
} // namespace smtk

#endif
