//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_pqSMTKCloseWithActiveOperationBehavior_h
#define smtk_extension_paraview_appcomponents_pqSMTKCloseWithActiveOperationBehavior_h

#include "smtk/operation/Manager.h"
#include "smtk/operation/Observer.h"

#include <atomic>

#include <QObject>

class pqSMTKWrapper;
class pqServer;

/// A behavior for prompting the user to cancel an application close if there is
/// an active operation..
class pqSMTKCloseWithActiveOperationBehavior : public QObject
{
  Q_OBJECT
  using Superclass = QObject;

public:
  static pqSMTKCloseWithActiveOperationBehavior* instance(QObject* parent = nullptr);
  ~pqSMTKCloseWithActiveOperationBehavior() override;

  static int showDialog(std::size_t numberOfActiveOperations);

protected Q_SLOTS:
  void trackActiveOperations(pqSMTKWrapper* wrapper, pqServer* server);

protected:
  pqSMTKCloseWithActiveOperationBehavior(QObject* parent = nullptr);

  smtk::operation::Observers::Key m_key;
  std::weak_ptr<smtk::operation::Manager> m_weakManager;

private:
  Q_DISABLE_COPY(pqSMTKCloseWithActiveOperationBehavior);
};

#endif // smtk_extension_paraview_appcomponents_pqSMTKCloseWithActiveOperationBehavior_h
