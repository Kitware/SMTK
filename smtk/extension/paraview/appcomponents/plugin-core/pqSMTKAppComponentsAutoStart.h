//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_pqSMTKAppComponentsAutoStart_h
#define smtk_extension_paraview_appcomponents_pqSMTKAppComponentsAutoStart_h

#include <QObject>

class pqSMTKWrapper;
class pqServer;
class vtkSMProxy;

class pqSMTKAppComponentsAutoStart : public QObject
{
  Q_OBJECT
  using Superclass = QObject;

  static vtkSMProxy* resourceManager();

public:
  pqSMTKAppComponentsAutoStart(QObject* parent = nullptr);
  ~pqSMTKAppComponentsAutoStart() override;

  void startup();
  void shutdown();

protected Q_SLOTS:
  /// Called whenever a new client-server connection is made and SMTK is enabled on the server.
  ///
  /// This is used to observe operations
  virtual void observeWrapper(pqSMTKWrapper* wrapper, pqServer* server);
  /// Called whenever a client-server disconnect is performed on an SMTK-enabled server.
  virtual void unobserveWrapper(pqSMTKWrapper* wrapper, pqServer* server);

protected:
  class pqInternal;
  pqInternal* m_p;

private:
  Q_DISABLE_COPY(pqSMTKAppComponentsAutoStart);
};

#endif
