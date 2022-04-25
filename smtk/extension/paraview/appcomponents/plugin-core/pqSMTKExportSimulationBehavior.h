//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_pqSMTKExportSimulationBehavior_h
#define smtk_extension_paraview_appcomponents_pqSMTKExportSimulationBehavior_h

#include "smtk/PublicPointerDefs.h"

#include "pqReaction.h"

#include <QObject>

class pqPipelineSource;
class pqServer;
class vtkSMReaderFactory;

/// A reaction for exporting an SMTK simulation.
class pqExportSimulationReaction : public pqReaction
{
  Q_OBJECT
  typedef pqReaction Superclass;

public:
  /**
  * Constructor. Parent cannot be nullptr.
  */
  pqExportSimulationReaction(QAction* parent);

  static void exportSimulation();

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
  void onTriggered() override { pqExportSimulationReaction::exportSimulation(); }

private:
  Q_DISABLE_COPY(pqExportSimulationReaction)
};

/// Create a menu item under "File" for exporting simulations. Currently,
/// simulations are python operations. The action associated with this menu item
/// loads a python operation, executes it immediately, and then unloads the
/// operation.
class pqSMTKExportSimulationBehavior : public QObject
{
  Q_OBJECT
  using Superclass = QObject;

public:
  static pqSMTKExportSimulationBehavior* instance(QObject* parent = nullptr);
  ~pqSMTKExportSimulationBehavior() override;

protected:
  pqSMTKExportSimulationBehavior(QObject* parent = nullptr);

private:
  Q_DISABLE_COPY(pqSMTKExportSimulationBehavior);
};

#endif // smtk_extension_paraview_appcomponents_pqSMTKExportSimulationBehavior_h
