//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_pqSMTKCloseResourceBehavior_h
#define smtk_extension_paraview_appcomponents_pqSMTKCloseResourceBehavior_h

#include "smtk/PublicPointerDefs.h"

#include "pqReaction.h"

#include <QObject>

class QMenu;

/// A reaction for closing an SMTK Resource.
class pqCloseResourceReaction : public pqReaction
{
  Q_OBJECT
  typedef pqReaction Superclass;

public:
  /**
  * Constructor. Parent cannot be nullptr.
  */
  pqCloseResourceReaction(QAction* parent);

  static void closeResource();

public Q_SLOTS:
  /**
  * Updates the enabled state. Applications need not explicitly call this.
  */
  void updateEnableState() override;

protected:
  /**
  * Called when the action is triggered.
  */
  void onTriggered() override { pqCloseResourceReaction::closeResource(); }

private:
  Q_DISABLE_COPY(pqCloseResourceReaction)
};

/// Create a menu item under "File" for closing an SMTK resource. The behavior
/// checks the state of the active resource and allows the user to save or
/// cancel the close action if the resource is modified from its on-disk
/// representation. If the resource is "clean", the user has selected to save
/// the resource or the user has opted to discard changes, the resource is then
/// removed from the resource manager.
class pqSMTKCloseResourceBehavior : public QObject
{
  Q_OBJECT
  using Superclass = QObject;

public:
  static pqSMTKCloseResourceBehavior* instance(QObject* parent = nullptr);
  ~pqSMTKCloseResourceBehavior() override;

protected:
  pqSMTKCloseResourceBehavior(QObject* parent = nullptr);

private:
  Q_DISABLE_COPY(pqSMTKCloseResourceBehavior);

  QMenu* m_newMenu{ nullptr };
};

#endif // smtk_extension_paraview_appcomponents_pqSMTKCloseResourceBehavior_h
