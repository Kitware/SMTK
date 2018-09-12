//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME girderlogindialog.h
// .SECTION Description
// .SECTION See Also

#ifndef girderfilebrowser_girderlogindialog_h
#define girderfilebrowser_girderlogindialog_h

#include <memory>

#include <QDialog>
#include <QString>

// Are we building a stand alone girder file browser?
#ifdef GIRDERFILEBROWSER_BUILD_STANDALONE
// Empty definition to prevent compilation error
#define SMTKCUMULUSEXT_EXPORT
#else
#include "smtk/extension/cumulus/Exports.h"
#endif

namespace Ui
{
class GirderLoginDialog;
}

namespace cumulus
{

class SMTKCUMULUSEXT_EXPORT GirderLoginDialog : public QDialog
{
  Q_OBJECT

public:
  GirderLoginDialog(QWidget* parent = nullptr);

  virtual ~GirderLoginDialog() override;

  void setApiUrl(const QString& url);
  void setUsername(const QString& name);

signals:
  // This should be connected to an authenticator
  void beginAuthentication(const QString& apiUrl, const QString& username, const QString& password);

public slots:
  void accept() override;
  void authenticationFailed(const QString& message = "");

private:
  std::unique_ptr<Ui::GirderLoginDialog> m_ui;
};

} // end namespace

#endif
