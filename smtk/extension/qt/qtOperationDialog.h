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

class QShowEvent;
class QWidget;
class qtOperationDialogInternals;

namespace smtk
{
namespace extension
{

// Modal dialog for smtk operation view.
class SMTKQTEXT_EXPORT qtOperationDialog : public QDialog
{
  Q_OBJECT

public:
  qtOperationDialog(smtk::operation::OperationPtr operation, smtk::view::ManagerPtr viewManager,
    QWidget* parentWidget);
  virtual ~qtOperationDialog();

signals:
  void operationExecuted(const smtk::operation::Operation::Result& result);

public slots:

protected slots:
  void onOperationExecuted(const smtk::operation::Operation::Result& result);

protected:
  // Override showEvent() in order to fix Qt sizing issue
  void showEvent(QShowEvent* event) override;

private:
  qtOperationDialogInternals* m_internals;
}; // class
}
}

#endif
