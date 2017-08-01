//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtViewInfoDialog - A Information Dialog for SMTK Operators
// .SECTION Description
// .SECTION Caveats
#ifndef _qtViewInfoDialog_h
#define _qtViewInfoDialog_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/extension/qt/Exports.h"
#include <QtWidgets/QDialog>

namespace Ui
{
class qtViewInfoDialog;
}

namespace smtk
{
namespace extension
{

class SMTKQTEXT_EXPORT qtViewInfoDialog : public QDialog
{
  Q_OBJECT

public:
  qtViewInfoDialog(QWidget* Parent);
  ~qtViewInfoDialog() override;
  qtViewInfoDialog(const qtViewInfoDialog&) = delete;
  qtViewInfoDialog& operator=(const qtViewInfoDialog&) = delete;

  void displayInfo(smtk::attribute::AttributePtr att);
  void displayInfo(smtk::common::ViewPtr view);

private:
  Ui::qtViewInfoDialog* m_dialog;
  smtk::common::ViewPtr m_view;
  smtk::attribute::AttributePtr m_attribute;
};
}
}
#endif // !_qtViewInfoDialog_h
