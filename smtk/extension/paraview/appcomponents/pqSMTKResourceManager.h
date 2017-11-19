//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponenets_pqSMTKResourceManager_h
#define smtk_extension_paraview_appcomponenets_pqSMTKResourceManager_h

#include "smtk/extension/paraview/appcomponents/Exports.h"

#include "pqProxy.h"

class pqOutputPort;

class SMTKPQCOMPONENTSEXT_EXPORT pqSMTKResourceManager : public pqProxy
{
  Q_OBJECT
  using Superclass = pqProxy;

public:
  pqSMTKResourceManager(const QString& regGroup, const QString& regName, vtkSMProxy* proxy,
    pqServer* server, QObject* parent = nullptr);
  ~pqSMTKResourceManager() override;

protected slots:
  virtual void paraviewSelectionChanged(pqOutputPort* port);

private:
  Q_DISABLE_COPY(pqSMTKResourceManager);
};

#endif
