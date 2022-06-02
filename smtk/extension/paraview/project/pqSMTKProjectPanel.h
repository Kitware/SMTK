//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_project_pqSMTKProjectPanel_h
#define smtk_extension_paraview_project_pqSMTKProjectPanel_h

#include "smtk/extension/paraview/project/smtkPQProjectExtModule.h"

#include "smtk/extension/paraview/project/pqSMTKProjectBrowser.h"
#include "smtk/extension/qt/qtUIManager.h"

#include <QPointer>
#include <QWidget>

class QVBoxLayout;

/**\brief A panel that displays SMTK projects available to the application/user.
  *
  */
class SMTKPQPROJECTEXT_EXPORT pqSMTKProjectPanel : public QWidget
{
  Q_OBJECT
  typedef QWidget Superclass;

public:
  pqSMTKProjectPanel(QWidget* parent = nullptr);
  ~pqSMTKProjectPanel() override;

  /// Let the panel display a custom view config, from json or xml.
  void setView(const smtk::view::ConfigurationPtr& view);

Q_SIGNALS:
  void titleChanged(QString title);

protected Q_SLOTS:
  virtual void sourceAdded(pqSMTKWrapper* mgr, pqServer* server);
  virtual void sourceRemoved(pqSMTKWrapper* mgr, pqServer* server);

protected:
  pqSMTKProjectBrowser* m_browser{ nullptr };
  smtk::view::ConfigurationPtr m_view;
  smtk::extension::qtUIManager* m_viewUIMgr{ nullptr };
  /// The central widget's layout.
  QPointer<QVBoxLayout> m_layout;
};

#endif
