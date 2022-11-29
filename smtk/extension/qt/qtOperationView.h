//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtOperationView - UI components for attribute Operation View
// .SECTION Description
// .SECTION See Also
// qtSection

#ifndef smtk_extension_qtOperationView_h
#define smtk_extension_qtOperationView_h

#include "smtk/operation/Operation.h"

#include "smtk/extension/qt/Exports.h"
#include "smtk/extension/qt/qtBaseAttributeView.h"

#include <QPointer>
#include <QThread>

class qtOperationViewInternals;
class QPushButton;

namespace smtk
{
namespace extension
{

class SMTKQTEXT_EXPORT qtOperationView : public qtBaseAttributeView
{
  Q_OBJECT

public:
  smtkTypenameMacro(qtOperationView);

  static qtBaseView* createViewWidget(const smtk::view::Information& info);

  qtOperationView(const smtk::view::Information& info);

  ~qtOperationView() override;

  /// True when the view is configured to launch operations when the apply button is pressed.
  bool runOperationOnApply() const;
  /// Set whether the view should launch operations or allow external launchers to do so.
  void setRunOperationOnApply(bool shouldLaunch);

  /// True when the apply button should be disabled after each run of the operation until
  /// a parameter is modified. This is true by default but some custom item views may
  /// turn this feature off.
  bool disableApplyAfterRun() const { return m_disableApply; }
  void setDisableApplyAfterRun(bool shouldDisable) { m_disableApply = shouldDisable; }

  QPointer<QPushButton> applyButton() const;
  QPointer<QPushButton> doneButton() const;
  const smtk::operation::OperationPtr& operation() const;
  void showInfoButton(bool visible = true);

  // Replaces default buttons, for embedding operation view in other widgets.
  // Can use nullptr for any argument to ignore that button completely.
  void setButtons(
    QPointer<QPushButton> applyButton,
    QPointer<QPushButton> infoButton,
    QPointer<QPushButton> doneButton);

  // Validates the view information to see if it is suitable for creating a qtOperationView instance
  static bool validateInformation(const smtk::view::Information& info);

public Q_SLOTS:
  void updateUI() override;
  void showAdvanceLevelOverlay(bool show) override;
  void onShowCategory() override;
  virtual void onModifiedParameters();
  virtual void onModifiedParameter(qtItem* item);
  virtual void onOperate();

Q_SIGNALS:
  /**\brief Signaled when the user presses the "Apply" button.
    *
    * Note that if runOperationOnApply() returns true, the operation
    * will have been launched when this is emitted. Otherwise, this
    * signal should be used by external agents to launch the operation.
    */
  void operationRequested(const smtk::operation::OperationPtr& brOp);
  /// Signaled when the user presses the "Done" button.
  void doneEditing();

  // Currently, the operation view is responsible for executing the operation.
  // Since some processes need to distinguish between results from the operation
  // view and results coming from other places, we signal the results of
  // operations specifically executed within this view.
  void operationExecuted(const smtk::operation::Operation::Result& result);

protected:
  void createWidget() override;
  void setInfoToBeDisplayed() override;
  bool m_applied;              // indicates if the current settings have been applied
  bool m_disableApply{ true }; // disable the Apply button until an item is modified?

private:
  qtOperationViewInternals* Internals;

}; // class
}; // namespace extension
}; // namespace smtk

#endif
