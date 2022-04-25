//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_project_plugin_pqSMTKProjectPanel_h
#define smtk_project_plugin_pqSMTKProjectPanel_h

#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/project/plugin/pqSMTKProjectBrowser.h"

#include <QDockWidget>

/**\brief A panel that displays SMTK projects available to the application/user.
  *
  */
class pqSMTKProjectPanel : public QDockWidget
{
  Q_OBJECT
  typedef QDockWidget Superclass;

public:
  pqSMTKProjectPanel(QWidget* parent = nullptr);
  ~pqSMTKProjectPanel() override;

  /// Let the panel display a custom view config, from json or xml.
  void setView(const smtk::view::ConfigurationPtr& view);

protected Q_SLOTS:
  virtual void sourceAdded(pqSMTKWrapper* mgr, pqServer* server);
  virtual void sourceRemoved(pqSMTKWrapper* mgr, pqServer* server);

protected:
  pqSMTKProjectBrowser* m_browser{ nullptr };
  smtk::view::ConfigurationPtr m_view;
  smtk::extension::qtUIManager* m_viewUIMgr{ nullptr };
};

#endif
