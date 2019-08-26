//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_pqSMTKNewResourceBehavior_h
#define smtk_extension_paraview_appcomponents_pqSMTKNewResourceBehavior_h

#include "smtk/extension/paraview/appcomponents/Exports.h"

#include "smtk/PublicPointerDefs.h"

#include "smtk/operation/GroupObserver.h"

#include "pqReaction.h"

#include <QObject>

class QMenu;

/// A reaction for creating a new SMTK Resource.
class pqNewResourceReaction : public pqReaction
{
  Q_OBJECT
  typedef pqReaction Superclass;

public:
  /**
  * Constructor. Parent cannot be NULL.
  */
  pqNewResourceReaction(const std::string& operationName, QAction* parent);

  void newResource();

protected:
  /**
  * Called when the action is triggered.
  */
  void onTriggered() override { this->newResource(); }

private:
  Q_DISABLE_COPY(pqNewResourceReaction)

  std::string m_operationName;
};

/// Create a menu item under "File" for creating a new SMTK resource. The
/// behavior constructs a client-side operation for the purposes of populating a
/// modal dialog with the create operation's parameters. These parameters are
/// then serialized to a json string, and a server-side operation is created and
/// executed using the deserialized parameters.
class SMTKPQCOMPONENTSEXT_EXPORT pqSMTKNewResourceBehavior : public QObject
{
  Q_OBJECT
  using Superclass = QObject;

public:
  static pqSMTKNewResourceBehavior* instance(QObject* parent = nullptr);
  ~pqSMTKNewResourceBehavior() override;

  QMenu* fileMenu();

  void setNewMenu(QMenu*);

public slots:
  void updateNewMenu();

protected:
  pqSMTKNewResourceBehavior(QObject* parent = nullptr);

private:
  Q_DISABLE_COPY(pqSMTKNewResourceBehavior);

  QMenu* m_newMenu;

  smtk::operation::GroupObservers::Key m_key;
};

#endif // smtk_extension_paraview_appcomponents_pqSMTKNewResourceBehavior_h
