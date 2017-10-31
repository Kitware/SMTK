//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_pqSMTKResource_h
#define smtk_extension_paraview_appcomponents_pqSMTKResource_h

#include "pqPipelineSource.h"

#include "smtk/extension/paraview/appcomponents/Exports.h"

class SMTKPQCOMPONENTSEXT_EXPORT pqSMTKResource : public pqPipelineSource
{
  Q_OBJECT
  typedef pqPipelineSource Superclass;

public:
  pqSMTKResource(const QString& grp, const QString& name, vtkSMProxy* proxy, pqServer* server,
    QObject* parent = nullptr);
  ~pqSMTKResource() override;

protected slots:
  virtual void synchronizeResource();
};
#endif // smtk_extension_paraview_appcomponents_pqSMTKResource_h
