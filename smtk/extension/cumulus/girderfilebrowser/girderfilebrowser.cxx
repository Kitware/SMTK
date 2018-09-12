//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include <cstdlib>
#include <iostream>
#include <memory>

#include <QApplication>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QString>

#include "girderauthenticator.h"
#include "ui/girderfilebrowserdialog.h"
#include "ui/girderlogindialog.h"

int main(int argc, char* argv[])
{
  QApplication app(argc, argv);

  std::unique_ptr<QNetworkAccessManager> networkManager(new QNetworkAccessManager);

  using cumulus::GirderLoginDialog;
  GirderLoginDialog loginDialog;

  using cumulus::GirderAuthenticator;
  GirderAuthenticator girderAuthenticator(networkManager.get());

  // Below is an example of creating a custom root folder.
  // It needs to match the girder information exactly, or the behavior is
  // undefined
  //  QMap<QString, QString> customRootFolder;
  //  customRootFolder["name"] = "Public";
  //  customRootFolder["id"] = "5b16b1fd8d777f15ebe1ffc9";
  //  customRootFolder["type"] = "folder";

  using cumulus::GirderFileBrowserDialog;
  GirderFileBrowserDialog gfbDialog(networkManager.get());

  // An example of how to change the item mode.
  //gfbDialog.setItemMode("Treat Items as Folders");

  QString apiUrl = std::getenv("GIRDER_API_URL");
  QString apiKey = std::getenv("GIRDER_API_KEY");

  if (!apiUrl.isEmpty())
    loginDialog.setApiUrl(apiUrl);

  // Attempt api key authentication if both environment variables are present
  std::unique_ptr<QMetaObject::Connection> tempCon;
  if (!apiUrl.isEmpty() && !apiKey.isEmpty())
  {
    tempCon.reset(new QMetaObject::Connection());
    girderAuthenticator.authenticateApiKey(apiUrl, apiKey);
    // If authentication fails, show the dialog, but only once.
    *tempCon = QObject::connect(&girderAuthenticator, &GirderAuthenticator::authenticationErrored,
      &loginDialog, [&loginDialog, &tempCon]() {
        loginDialog.show();
        tempCon.reset();
      });
  }
  else
  {
    loginDialog.show();
  }

  // Connect the "ok" button of the login dialog to the girder authenticator
  QObject::connect(&loginDialog, &GirderLoginDialog::beginAuthentication, &girderAuthenticator,
    &GirderAuthenticator::authenticatePassword);
  // If authentication fails, print a message on the login dialog
  QObject::connect(&girderAuthenticator, &GirderAuthenticator::authenticationErrored, &loginDialog,
    &GirderLoginDialog::authenticationFailed);
  // If authentication fails, also print it to the terminal
  QObject::connect(&girderAuthenticator, &GirderAuthenticator::authenticationErrored,
    [](const QString& errorMessage) { std::cerr << errorMessage.toStdString() << "\n"; });

  // If we succeed in authenticating, hide the login dialog, set the
  // girder token, and show the browser window.
  QObject::connect(&girderAuthenticator, &GirderAuthenticator::authenticationSucceeded,
    &loginDialog, &GirderLoginDialog::hide);
  QObject::connect(&girderAuthenticator, &GirderAuthenticator::authenticationSucceeded, &gfbDialog,
    &GirderFileBrowserDialog::setApiUrlAndGirderToken);
  QObject::connect(&girderAuthenticator, &GirderAuthenticator::authenticationSucceeded, &gfbDialog,
    &GirderFileBrowserDialog::begin);
  QObject::connect(&girderAuthenticator, &GirderAuthenticator::authenticationSucceeded, &gfbDialog,
    &GirderFileBrowserDialog::show);

  // Just a simple demonstration of how an object can be chosen
  QObject::connect(&gfbDialog, &GirderFileBrowserDialog::objectChosen,
    [](const QMap<QString, QString>& objectInfo) {
      qDebug() << "\n*** Object chosen ***";
      qDebug() << "name:" << objectInfo["name"];
      qDebug() << "id:" << objectInfo["id"];
      qDebug() << "type:" << objectInfo["type"];
    });

  return app.exec();
}
