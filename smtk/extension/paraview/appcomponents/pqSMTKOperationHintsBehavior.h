//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_pqSMTKOperationHintsBehavior_h
#define smtk_extension_paraview_appcomponents_pqSMTKOperationHintsBehavior_h

#include "smtk/extension/paraview/appcomponents/smtkPQComponentsExtModule.h"

#include "smtk/view/Selection.h"

#include "smtk/operation/Observer.h"
#include "smtk/operation/Operation.h"

#include "smtk/extension/paraview/appcomponents/pqQtKeywordWrapping.h"

#include <QObject>

class pqSMTKWrapper;
class pqServer;

/**\brief An operation observer to apply hints from operation results.
  *
  * This installs an operation observer on each pqSMTKWrapper's operation manager.
  * Whenever an operation is run, the observer examines hints referenced by the
  * the operation's results and takes action.
  */
class SMTKPQCOMPONENTSEXT_EXPORT pqSMTKOperationHintsBehavior : public QObject
{
  Q_OBJECT
  using Superclass = QObject;

public:
  pqSMTKOperationHintsBehavior(QObject* parent = nullptr);
  ~pqSMTKOperationHintsBehavior() override;

  static pqSMTKOperationHintsBehavior* instance(QObject* parent);

  /// Called when an operation completes in order to apply hints in its result.
  int processHints(
    const smtk::operation::Operation& operation,
    smtk::operation::EventType event,
    smtk::operation::Operation::Result result);

  /// Called when an application selection changes in order to remove ephemera.
  void removeEphemera(const std::string& changeSource, smtk::view::Selection::Ptr selection);

protected Q_SLOTS:
  /// Called when a new client-server connection is added.
  virtual void observeWrapper(pqSMTKWrapper*, pqServer*);

  /// Called when a client-server connection is removed.
  virtual void unobserveWrapper(pqSMTKWrapper*, pqServer*);

protected:
  class pqInternal;
  pqInternal* m_p;

private:
  Q_DISABLE_COPY(pqSMTKOperationHintsBehavior);
};

#endif
