//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_pqSMTKImportIntoResourceBehavior_h
#define smtk_extension_paraview_appcomponents_pqSMTKImportIntoResourceBehavior_h

#include "smtk/PublicPointerDefs.h"

#include "pqReaction.h"

#include <QObject>

class QMenu;

/// A reaction for creating a new SMTK Resource.
class pqImportIntoResourceReaction : public pqReaction
{
  Q_OBJECT
  typedef pqReaction Superclass;

public:
  /**
  * Constructor. Parent cannot be nullptr.
  */
  pqImportIntoResourceReaction(QAction* parent);

  void importIntoResource();

public Q_SLOTS:
  /**
  * Updates the enabled state. Applications need not explicitly call
  * this.
  */
  void updateEnableState() override;

protected:
  /**
  * Called when the action is triggered.
  */
  void onTriggered() override { this->importIntoResource(); }

private:
  Q_DISABLE_COPY(pqImportIntoResourceReaction)
};

/// Create a menu item under "File" for importing a file into an existing SMTK
/// resource.
class pqSMTKImportIntoResourceBehavior : public QObject
{
  Q_OBJECT
  using Superclass = QObject;

public:
  static pqSMTKImportIntoResourceBehavior* instance(QObject* parent = nullptr);
  ~pqSMTKImportIntoResourceBehavior() override;

  QMenu* fileMenu();

  void setImportIntoMenu(QMenu*);

protected:
  pqSMTKImportIntoResourceBehavior(QObject* parent = nullptr);

private:
  Q_DISABLE_COPY(pqSMTKImportIntoResourceBehavior);
};

#endif // smtk_extension_paraview_appcomponents_pqSMTKImportIntoResourceBehavior_h
