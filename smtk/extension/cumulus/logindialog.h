//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME logindialog.h
// .SECTION Description
// .SECTION See Also

#ifndef __smtk_extension_cumulus_logindialog_h
#define __smtk_extension_cumulus_logindialog_h

#include <QDialog>

namespace Ui {
class LoginDialog;
}

namespace cumulus
{


class LoginDialog: public QDialog
{
  Q_OBJECT

public:
  explicit LoginDialog(QWidget *parentObject = 0);
  ~LoginDialog();

  void setErrorMessage(const QString &message);

public slots:
  void accept();
  void reject();

signals:
  void entered(const QString &username, const QString &password);
  void canceled();

private:
  Ui::LoginDialog *ui;

};

} // end namespace

#endif
