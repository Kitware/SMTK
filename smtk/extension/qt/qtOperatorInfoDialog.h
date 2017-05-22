//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtOperatorInfoDialog - A Information Dialog for SMTK Operators
// .SECTION Description
// .SECTION Caveats
#ifndef _qtOperatorInfoDialog_h
#define _qtOperatorInfoDialog_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/extension/qt/Exports.h"
#include <QDialog>

namespace Ui
{
class qtOperatorInfoDialog;
}

namespace smtk
{
namespace extension
{

class SMTKQTEXT_EXPORT qtOperatorInfoDialog : public QDialog
{
  Q_OBJECT

public:
  qtOperatorInfoDialog(QWidget* Parent);
  ~qtOperatorInfoDialog() override;

  void displayOperator(smtk::model::OperatorPtr op);

private:
  qtOperatorInfoDialog(const qtOperatorInfoDialog&);
  qtOperatorInfoDialog& operator=(const qtOperatorInfoDialog&);

  Ui::qtOperatorInfoDialog* m_dialog;
  smtk::model::OperatorPtr m_op;
};
}
}
#endif // !_qtOperatorInfoDialog_h
