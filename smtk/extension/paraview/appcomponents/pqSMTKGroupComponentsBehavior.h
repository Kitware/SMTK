//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_pqSMTKGroupComponentsBehavior_h
#define smtk_extension_paraview_appcomponents_pqSMTKGroupComponentsBehavior_h

#include "smtk/extension/paraview/appcomponents/smtkPQComponentsExtModule.h"

#include "smtk/PublicPointerDefs.h"

#include "pqReaction.h"

#include "smtk/extension/paraview/appcomponents/pqQtKeywordWrapping.h"
#include "smtk/view/SelectionObserver.h"

#include <QObject>

class pqServer;

class QMenu;

/// A menu-item reaction for grouping currently-selected components.
class SMTKPQCOMPONENTSEXT_EXPORT pqGroupComponentsReaction : public pqReaction
{
  Q_OBJECT
  typedef pqReaction Superclass;

public:
  /**
    * Constructor. Parent cannot be nullptr.
    */
  pqGroupComponentsReaction(QAction* parent, bool isGrouping);

  static void groupSelectedComponents();
  static void ungroupSelectedComponents();

public Q_SLOTS:
  /// Mark the action enabled when selection is appropriate and disabled otherwise.
  void updateEnableState() override;

  /// When the active server changes, listen to its corresponding SMTK selection.
  void updateSelectionObserver(pqServer* server);

protected:
  /**
    * Called when the action is triggered.
    */
  void onTriggered() override
  {
    if (m_isGrouping)
    {
      pqGroupComponentsReaction::groupSelectedComponents();
    }
    else
    {
      pqGroupComponentsReaction::ungroupSelectedComponents();
    }
  }

  /// Whether the current action causes grouping (true) or ungrouping (false)
  bool m_isGrouping;
  /// The observer key for the currently-observed selection.
  smtk::view::SelectionObservers::Key m_observerKey;

private:
  Q_DISABLE_COPY(pqGroupComponentsReaction)
};

/// Create a menu item under "Edit" for grouping selected components.
class SMTKPQCOMPONENTSEXT_EXPORT pqSMTKGroupComponentsBehavior : public QObject
{
  Q_OBJECT
  using Superclass = QObject;

public:
  static pqSMTKGroupComponentsBehavior* instance(QObject* parent = nullptr);
  ~pqSMTKGroupComponentsBehavior() override;

protected:
  pqSMTKGroupComponentsBehavior(QObject* parent = nullptr);

private:
  Q_DISABLE_COPY(pqSMTKGroupComponentsBehavior);

  QMenu* m_newMenu{ nullptr };
};

#endif // smtk_extension_paraview_appcomponents_pqSMTKGroupComponentsBehavior_h
