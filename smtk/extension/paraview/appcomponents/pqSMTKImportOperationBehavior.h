//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_pqSMTKImportOperationBehavior_h
#define smtk_extension_paraview_appcomponents_pqSMTKImportOperationBehavior_h

#include "smtk/extension/paraview/appcomponents/smtkPQComponentsExtModule.h"

#include "smtk/PublicPointerDefs.h"

#include "pqReaction.h"

#include "smtk/extension/paraview/appcomponents/fixWrap.h"

#include <QObject>

class pqPipelineSource;
class pqServer;
class vtkSMReaderFactory;

/// A reaction for importing an SMTKf operation.
class SMTKPQCOMPONENTSEXT_EXPORT pqImportOperationReaction : public pqReaction
{
  Q_OBJECT
  typedef pqReaction Superclass;

public:
  /**
  * Constructor. Parent cannot be nullptr.
  */
  pqImportOperationReaction(QAction* parent);

  static void importOperation();

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
  void onTriggered() override { pqImportOperationReaction::importOperation(); }

private:
  Q_DISABLE_COPY(pqImportOperationReaction)
};

/// Create a menu item under "File" for importing operations.
class SMTKPQCOMPONENTSEXT_EXPORT pqSMTKImportOperationBehavior : public QObject
{
  Q_OBJECT
  using Superclass = QObject;

public:
  static pqSMTKImportOperationBehavior* instance(QObject* parent = nullptr);
  ~pqSMTKImportOperationBehavior() override;

protected:
  pqSMTKImportOperationBehavior(QObject* parent = nullptr);

private:
  Q_DISABLE_COPY(pqSMTKImportOperationBehavior);
};

#endif // smtk_extension_paraview_appcomponents_pqSMTKImportOperationBehavior_h
