//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_pqSMTKSaveResourceBehavior_h
#define smtk_extension_paraview_appcomponents_pqSMTKSaveResourceBehavior_h

#include "smtk/extension/paraview/appcomponents/smtkPQComponentsExtModule.h"

#include "smtk/PublicPointerDefs.h"

#include "pqReaction.h"

#include "smtk/extension/paraview/appcomponents/pqQtKeywordWrapping.h"

#include <QObject>

class pqPipelineSource;
class pqServer;
class pqSMTKResource;
class vtkSMReaderFactory;

/// A reaction for saving an SMTK resource to its internal location.
class SMTKPQCOMPONENTSEXT_EXPORT pqSaveResourceReaction : public pqReaction
{
  Q_OBJECT
  typedef pqReaction Superclass;

public:
  /**
  * Constructor. Parent cannot be nullptr.
  */
  pqSaveResourceReaction(QAction* parent);

  enum class State
  {
    Succeeded,
    Failed,
    Aborted
  };

  /// Save the resource to disk (or if null, save the active object's resource).
  static State saveResource(pqSMTKResource* smtkResource = nullptr);
  /// Save the resource to disk (called by the variant above).
  static State saveResource(
    const std::shared_ptr<smtk::resource::Resource>& resource,
    const std::shared_ptr<smtk::common::Managers>& managers);

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
  void onTriggered() override { (void)pqSaveResourceReaction::saveResource(); }

private:
  Q_DISABLE_COPY(pqSaveResourceReaction)
};

/// A reaction for saving an SMTK resource to a user-defined location.
class SMTKPQCOMPONENTSEXT_EXPORT pqSaveResourceAsReaction : public pqReaction
{
  Q_OBJECT
  typedef pqReaction Superclass;

public:
  /**
  * Constructor. Parent cannot be nullptr.
  */
  pqSaveResourceAsReaction(QAction* parent);

  /// Save the resource to disk under a different name (or if null, the active object's resource).
  static pqSaveResourceReaction::State saveResourceAs(pqSMTKResource* smtkResource = nullptr);
  /// Save the resource to disk under a different name (called by the variant above).
  static pqSaveResourceReaction::State saveResourceAs(
    const std::shared_ptr<smtk::resource::Resource>& resource,
    const std::shared_ptr<smtk::common::Managers>& managers);

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
  void onTriggered() override { (void)pqSaveResourceAsReaction::saveResourceAs(); }

private:
  Q_DISABLE_COPY(pqSaveResourceAsReaction)
};

/// Create a menu item under "File" for saving resources.
class SMTKPQCOMPONENTSEXT_EXPORT pqSMTKSaveResourceBehavior : public QObject
{
  Q_OBJECT
  using Superclass = QObject;

public:
  static pqSMTKSaveResourceBehavior* instance(QObject* parent = nullptr);
  ~pqSMTKSaveResourceBehavior() override;

protected:
  pqSMTKSaveResourceBehavior(QObject* parent = nullptr);

private:
  Q_DISABLE_COPY(pqSMTKSaveResourceBehavior);
};

#endif // smtk_extension_paraview_appcomponents_pqSMTKSaveResourceBehavior_h
