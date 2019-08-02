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

#ifndef __smtk_extension_qtOperationView_h
#define __smtk_extension_qtOperationView_h

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
class SMTKQTEXT_EXPORT OperationViewInfo : public ViewInfo
{
public:
  OperationViewInfo(smtk::view::ViewPtr view, smtk::operation::OperationPtr targetOperation,
    QWidget* parent, qtUIManager* uiman)
    : ViewInfo(view, parent, uiman)
    , m_operator(targetOperation)
  {
  }

  OperationViewInfo(smtk::view::ViewPtr view, smtk::operation::OperationPtr targetOperation,
    QWidget* parent, qtUIManager* uiman, const std::map<std::string, QLayout*>& layoutDict)
    : ViewInfo(view, parent, uiman, layoutDict)
    , m_operator(targetOperation)
  {
  }

  OperationViewInfo() {}
  smtk::operation::OperationPtr m_operator;
};

class SMTKQTEXT_EXPORT qtOperationView : public qtBaseAttributeView
{
  Q_OBJECT

public:
  static qtBaseView* createViewWidget(const ViewInfo& info);

  qtOperationView(const OperationViewInfo& info);
  virtual ~qtOperationView();

  QPointer<QPushButton> applyButton() const;
  smtk::operation::OperationPtr operation() const;

public slots:
  void showAdvanceLevelOverlay(bool show) override;
  void requestModelEntityAssociation() override;
  void onShowCategory() override { this->updateAttributeData(); }
  virtual void onModifiedParameters();
  virtual void onModifiedParameter(qtItem* item);
  virtual void onOperate();
  void onOperationExecuted(const smtk::operation::Operation::Result& result);

signals:
  void operationRequested(const smtk::operation::OperationPtr& brOp);

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
}; // namespace attribute
}; // namespace smtk

#endif
