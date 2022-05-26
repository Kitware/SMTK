//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_pqSMTKDock_h
#define smtk_extension_paraview_appcomponents_pqSMTKDock_h

#include "smtk/extension/paraview/appcomponents/smtkPQComponentsExtModule.h"

#include "smtk/PublicPointerDefs.h"

#include "smtk/extension/paraview/appcomponents/pqQtKeywordWrapping.h"

#include <QDockWidget>
#include <QPointer>

/**\brief A dock widget that displays a panel.
  *
  */
class SMTKPQCOMPONENTSEXT_EXPORT pqSMTKDockBase : public QDockWidget
{
  Q_OBJECT
  typedef QDockWidget Superclass;

public:
  pqSMTKDockBase(QWidget* parent = nullptr);
  ~pqSMTKDockBase() override;

public Q_SLOTS:
  /// change our title
  void changeTitle(QString title) { this->setWindowTitle(title); }

protected:
  void setupPanel()
  {
    this->setWidget(m_panel);
    this->setWindowTitle(m_panel->windowTitle());
  }
  QPointer<QWidget> m_panel;
};

/**\brief A templated dock widget that displays the type of panel provided.
  *
  * Panels should implement the signal `void setTitle(QString title)` and
  * call `this->setWindowTitle(title)` to pass this class the dock window
  * title.
  */
template<class panelT>
class pqSMTKDock : public pqSMTKDockBase // don't _EXPORT because its templated.
{
  typedef pqSMTKDockBase Superclass;

public:
  pqSMTKDock(QString objName, QWidget* parent = nullptr)
    : Superclass(parent)
  {
    this->setObjectName(objName);
    auto* panel = new panelT(this);
    m_panel = panel;
    this->setupPanel();
    // panel must implement the setTitle signal.
    QObject::connect(panel, &panelT::titleChanged, this, &pqSMTKDock::changeTitle);
  }
  ~pqSMTKDock() override = default;
};

#endif // smtk_extension_paraview_appcomponents_pqSMTKDock_h
