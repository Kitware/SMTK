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

namespace smtk
{
namespace extension
{

class qtUIManager;

/**\brief Provides a dialog for launching SMTK operations.
 *
 * The intended use is for modelbuilder plugins that use menu or similar actions to invoke
 * SMTK operations. (For example, export operations are typically run from a modal dialog.)
 * The dialog is created as a QTabWidget with 2 tabs. The first tab embeds a qtOperationView
 * for the operation, and the second tab displays the operation's "info" content. The
 * dialog replaces and hides the "Apply", "Info" and "Cancel" buttons and consumes their Qt
 * connections. The dialog can be shown non-modal, to allow repeated running of an operation.
 * In non-modal operation, an optional "Apply & Close" button can be shown.
*/
class SMTKQTEXT_EXPORT qtOperationDialog : public QDialog
{
  Q_OBJECT

public:
  qtOperationDialog(
    smtk::operation::OperationPtr operation,
    QSharedPointer<smtk::extension::qtUIManager> uiManager,
    QWidget* parentWidget = nullptr,
    bool showApplyAndClose = false);
  qtOperationDialog(
    smtk::operation::OperationPtr operation,
    smtk::resource::ManagerPtr resourceManager,
    smtk::view::ManagerPtr viewManager,
    QWidget* parentWidget = nullptr,
    bool showApplyAndClose = false);

  /// Use this constructor to display the operation view in a
  /// vertically-scrolling area. You would generally only need
  /// this option if the operation parameters are lengthy and
  /// take up a significant amount of vertical display space.
  /// When setting the scrollable option, you would generally
  /// also want to call this class' setMinimumHeight() method.
  qtOperationDialog(
    smtk::operation::OperationPtr operation,
    smtk::resource::ManagerPtr resourceManager,
    smtk::view::ManagerPtr viewManager,
    bool scrollable,
    QWidget* parentWidget = nullptr);
  ~qtOperationDialog() override;

  /// forwarded to internal qtOperationView
  const smtk::operation::OperationPtr& operation() const;

Q_SIGNALS:
  void operationExecuted(const smtk::operation::Operation::Result& result);

public Q_SLOTS:
  /// forwarded to internal qtOperationView
  void updateUI();

protected Q_SLOTS:
  void onOperationExecuted(const smtk::operation::Operation::Result& result);

protected:
  void buildUI(
    smtk::operation::OperationPtr op,
    QSharedPointer<smtk::extension::qtUIManager> uiMManager,
    bool scrollable = false,
    bool showApplyAndClose = false);

  // Override showEvent() in order to fix Qt sizing issue
  void showEvent(QShowEvent* event) override;

private:
  qtOperationDialogInternals* m_internals;
}; // class
} // namespace extension
} // namespace smtk

#endif
