//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_pqSMTKProjectMenu_h
#define smtk_extension_paraview_appcomponents_pqSMTKProjectMenu_h

#include "smtk/extension/paraview/project/smtkPQProjectExtModule.h"

#include "smtk/PublicPointerDefs.h"

#include "pqReaction.h"

#include "smtk/extension/paraview/appcomponents/pqQtKeywordWrapping.h"

#include <QObject>

class QMenu;

/// A reaction for creating a new SMTK Project.
class SMTKPQPROJECTEXT_EXPORT pqNewProjectReaction : public pqReaction
{
  Q_OBJECT
  typedef pqReaction Superclass;

public:
  /**
  * Constructor. Parent cannot be nullptr.
  */
  pqNewProjectReaction(QAction* parent);

  void newProject();

protected:
  /**
  * Called when the action is triggered.
  */
  void onTriggered() override { this->newProject(); }

private:
  Q_DISABLE_COPY(pqNewProjectReaction)
};

/// Create a menu item under "File" for creating a new SMTK project. The
/// behavior constructs a client-side operation for the purposes of populating a
/// modal dialog with the create operation's parameters. These parameters are
/// then serialized to a json string, and a server-side operation is created and
/// executed using the deserialized parameters.
class SMTKPQPROJECTEXT_EXPORT pqSMTKProjectMenu : public QObject
{
  Q_OBJECT
  using Superclass = QObject;

public:
  static pqSMTKProjectMenu* instance(QObject* parent = nullptr);
  ~pqSMTKProjectMenu() override;

protected:
  pqSMTKProjectMenu(QObject* parent = nullptr);

private:
  Q_DISABLE_COPY(pqSMTKProjectMenu);
};

#endif // smtk_extension_paraview_appcomponents_pqSMTKProjectMenu_h
