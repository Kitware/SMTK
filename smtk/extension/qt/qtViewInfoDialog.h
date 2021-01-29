//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtViewInfoDialog - A Information Dialog for SMTK Operations
// .SECTION Description
// .SECTION Caveats
#ifndef _qtViewInfoDialog_h
#define _qtViewInfoDialog_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/extension/qt/Exports.h"
#include <QDialog>
#include <QString>

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
  void displayInfo(smtk::view::ConfigurationPtr view);

  static void formatInfoHtml(smtk::attribute::AttributePtr att, QString& html);

private:
  Ui::qtViewInfoDialog* m_dialog;
  smtk::view::ConfigurationPtr m_view;
  smtk::attribute::AttributePtr m_attribute;
};
}
}
#endif // !_qtViewInfoDialog_h
