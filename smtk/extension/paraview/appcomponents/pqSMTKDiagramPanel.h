//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_pqSMTKDiagramPanel_h
#define smtk_extension_paraview_appcomponents_pqSMTKDiagramPanel_h

#include "smtk/extension/paraview/appcomponents/smtkPQComponentsExtModule.h"
#include "smtk/view/UIElementState.h"

#include "smtk/extension/qt/diagram/qtDiagram.h"
#include "smtk/extension/qt/qtUIManager.h"

#include "smtk/string/Token.h"

#include "smtk/common/Deprecation.h"

#include <QPointer>
#include <QWidget>

#include "smtk/extension/paraview/appcomponents/pqQtKeywordWrapping.h"

class pqSMTKWrapper;
class pqServer;
class QVBoxLayout;

/**\brief A panel that displays SMTK tasks available to the user.
  *
  */
class SMTKPQCOMPONENTSEXT_EXPORT pqSMTKDiagramPanel
  : public QWidget
  , public smtk::view::UIElementState
{
  Q_OBJECT
  typedef QWidget Superclass;

public:
  pqSMTKDiagramPanel(QWidget* parent = nullptr);
  ~pqSMTKDiagramPanel() override;

  /// Let the panel display a custom view config, from json or xml.
  void setView(const smtk::view::ConfigurationPtr& view);

  /// Access the underlying resource browser.
  smtk::extension::qtDiagram* diagram() const { return m_diagram; }

  /// Return an (application-unique) token for the type of user-interface
  /// element this state object will serialize/deserialize.
  smtk::string::Token elementType() const override { return "pqSMTKDiagramPanel"; }

  /// Return the UI element's current, in-memory state to be serialized.
  nlohmann::json configuration() override;

  /// Using the deserialized configuration \a data, configure the user
  /// interface element to match it.
  bool configure(const nlohmann::json& data) override;

Q_SIGNALS:
  void titleChanged(QString title);

public Q_SLOTS:
  /// Bring this panel into focus (showing and raising it as required).
  virtual void focusPanel();

protected Q_SLOTS:
  virtual void resourceManagerAdded(pqSMTKWrapper* mgr, pqServer* server);
  virtual void resourceManagerRemoved(pqSMTKWrapper* mgr, pqServer* server);

protected:
  smtk::extension::qtDiagram* m_diagram{ nullptr };
  smtk::view::ConfigurationPtr m_view;
  smtk::extension::qtUIManager* m_viewUIMgr{ nullptr };
  /// The central widget's layout.
  QPointer<QVBoxLayout> m_layout;
};

class SMTK_DEPRECATED_IN_24_01("Use pqSMTKDiagramPanel instead.")
  SMTKPQCOMPONENTSEXT_EXPORT pqSMTKTaskPanel : public pqSMTKDiagramPanel
{
};

#endif // smtk_extension_paraview_appcomponents_pqSMTKDiagramPanel_h
