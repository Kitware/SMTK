//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtModelOperationWidget - the Model Operations Widget
// .SECTION Description
// .SECTION See Also
// qtSection

#ifndef __smtk_extension_qtModelOperationWidget_h
#define __smtk_extension_qtModelOperationWidget_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/extension/qt/Exports.h"
#include "smtk/operation/Operation.h"
#include <QWidget>

class qtModelOperationWidgetInternals;
class QAbstractButton;

namespace smtk
{
namespace io
{
class Logger;
}
namespace extension
{
class qtBaseView;
class qtFileItem;

class SMTKQTEXT_EXPORT qtModelOperationWidget : public QWidget
{
  Q_OBJECT

public:
  qtModelOperationWidget(QWidget* p = NULL);
  virtual ~qtModelOperationWidget();
  virtual void setSession(smtk::model::SessionPtr session);
  virtual void refreshOperationList();
  QSize sizeHint() const override;
  virtual smtk::operation::OperationPtr existingOperation(const std::string& opname);
  virtual qtBaseView* existingOperationView(const std::string& opname);

public slots:
  virtual bool setCurrentOperation(const std::string& opName, smtk::model::SessionPtr session);
  virtual bool initOperationUI(const smtk::operation::OperationPtr& brOp);
  virtual void expungeEntities(const smtk::model::EntityRefs& expungedEnts);
  virtual void onOperate();
  virtual void setOperationTargetActive(const smtk::common::UUID& eid)
  {
    emit activateOperationTarget(eid);
  }
  virtual void displayResult(const smtk::io::Logger& html);
  virtual void resetUI();
  virtual bool showPreviousOp();
  virtual void showLogInfo(bool visibilityMode);

signals:
  void operationRequested(const smtk::operation::OperationPtr& brOp);
  void operationCancelled(const smtk::operation::OperationPtr& brOp);
  void operationFinished(const smtk::operation::Operation::Result&);
  void fileItemCreated(smtk::extension::qtFileItem* fileItem);
  void activateOperationTarget(const smtk::common::UUID&);
  void broadcastExpungeEntities(const smtk::model::EntityRefs& expungedEnts);
  void operatorSet(const smtk::operation::OperationPtr& brOp);

protected slots:
  virtual void onOperationSelected();
  virtual void cancelCurrentOperation();
  virtual void cancelOperation(const std::string& opName);
  virtual bool checkExistingOperation(const std::string& opName);

protected:
  virtual void initWidget();

private:
  qtModelOperationWidgetInternals* Internals;

}; // class
}; // namespace model
}; // namespace smtk

#endif
