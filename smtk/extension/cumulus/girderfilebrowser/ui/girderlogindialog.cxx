//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "girderlogindialog.h"
#include "ui_girderlogindialog.h"

namespace cumulus
{

GirderLoginDialog::GirderLoginDialog(QWidget* parent)
  : QDialog(parent)
  , m_ui(new Ui::GirderLoginDialog)
{
  m_ui->setupUi(this);
}

GirderLoginDialog::~GirderLoginDialog() = default;

void GirderLoginDialog::setApiUrl(const QString& url)
{
  m_ui->edit_apiUrl->setText(url);
}

void GirderLoginDialog::setUsername(const QString& name)
{
  m_ui->edit_username->setText(name);
}

void GirderLoginDialog::accept()
{
  QString apiUrl = m_ui->edit_apiUrl->text();
  QString username = m_ui->edit_username->text();
  QString password = m_ui->edit_password->text();

  emit beginAuthentication(apiUrl, username, password);
}

void GirderLoginDialog::authenticationFailed(const QString& message)
{
  QString simplifiedMessage;
  if (message.contains("Login failed"))
    simplifiedMessage = "Login failed";
  else if (message.contains("Nothing matches the given URI"))
    simplifiedMessage = "Server could not be reached";
  else if (message.trimmed().endsWith("Response from server was:"))
    simplifiedMessage = "Server could not be reached";
  else if (message.contains("You don't have a password"))
    simplifiedMessage = "Your account doesn't have a password";
  else if (message.contains("Invalid API key"))
    simplifiedMessage = "Invalid API key";
  else
    simplifiedMessage = "Unknown error. Check terminal output.";

  // Make the message red.
  QString msgTemplate = tr("<font color='%1'>%2</font>");
  QString color = "red";
  m_ui->label_message->setText(msgTemplate.arg(color).arg(simplifiedMessage));
}

} // end namespace
