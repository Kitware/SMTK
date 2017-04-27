//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "logindialog.h"
#include "ui_logindialog.h"

namespace cumulus
{

LoginDialog::LoginDialog(QWidget* parentObject)
  : QDialog(parentObject)
  , ui(new Ui::LoginDialog)
{
  ui->setupUi(this);

  connect(ui->buttonBox, SIGNAL(rejected()), this, SIGNAL(canceled()));
}

LoginDialog::~LoginDialog()
{
  delete ui;
}

void LoginDialog::accept()
{
  emit entered(ui->usernameEdit->text(), ui->credentialsEdit->text());
  ui->credentialsEdit->clear();
  ui->messageLabel->clear();
  QDialog::accept();
}

void LoginDialog::reject()
{
  ui->credentialsEdit->clear();
  ui->messageLabel->clear();
  QDialog::reject();
}

void LoginDialog::setErrorMessage(const QString& message)
{
  ui->messageLabel->setText(message);
}

} // end namespace
