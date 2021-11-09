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

  QPointer<QPushButton> applyButton() const;
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

public slots:
  void updateUI() override;
  void showAdvanceLevelOverlay(bool show) override;
  void requestModelEntityAssociation() override;
  void onShowCategory() override;
  virtual void onModifiedParameters();
  virtual void onModifiedParameter(qtItem* item);
  virtual void onOperate();
  void onOperationExecuted(const smtk::operation::Operation::Result& result);

signals:
  void operationRequested(const smtk::operation::OperationPtr& brOp);
  void doneEditing();

  // Currently, the operation view is responsible for executing the operation.
  // Since some processes need to distinguish between results from the operation
  // view and results coming from other places, we signal the results of
  // operations specifically executed within this view.
  void operationExecuted(const smtk::operation::Operation::Result& result);

protected:
  void createWidget() override;
  void setInfoToBeDisplayed() override;
  bool m_applied; // indicates if the current settings have been applied

private:
  qtOperationViewInternals* Internals;

}; // class
}; // namespace extension
}; // namespace smtk

#endif
