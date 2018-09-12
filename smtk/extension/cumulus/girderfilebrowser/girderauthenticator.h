//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME girderauthenticator.h
// .SECTION Description
// .SECTION See Also

#ifndef girderfilebrowser_girderauthenticator_h
#define girderfilebrowser_girderauthenticator_h

#include <memory>

#include <QObject>
#include <QString>

// Are we building a stand alone girder file browser?
#ifdef GIRDERFILEBROWSER_BUILD_STANDALONE
// Empty definition to prevent compilation error
#define SMTKCUMULUSEXT_EXPORT
#else
#include "smtk/extension/cumulus/Exports.h"
#endif

class QNetworkAccessManager;
class QNetworkReply;

namespace cumulus
{

class SMTKCUMULUSEXT_EXPORT GirderAuthenticator : public QObject
{
  Q_OBJECT

public:
  GirderAuthenticator(QNetworkAccessManager* networkManager, QObject* parent = nullptr);

  virtual ~GirderAuthenticator() override;

  void authenticateApiKey(const QString& apiUrl, const QString& apiKey);
  void authenticatePassword(
    const QString& apiUrl, const QString& username, const QString& password);

signals:
  void authenticationSucceeded(const QString& apiUrl, const QString& girderToken);
  void authenticationErrored(const QString& errorMessage);

private slots:
  void finishAuthentication();

private:
  QNetworkAccessManager* m_networkManager;
  QString m_apiUrl;
  std::unique_ptr<QNetworkReply> m_pendingReply;
};

} // end namespace

#endif
