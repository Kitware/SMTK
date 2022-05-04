//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_pqSMTKRegisterImportersBehavior_h
#define smtk_extension_paraview_appcomponents_pqSMTKRegisterImportersBehavior_h

#include "smtk/extension/paraview/appcomponents/smtkPQComponentsExtModule.h"

#include "smtk/extension/paraview/appcomponents/pqQtKeywordWrapping.h"

#include <QObject>

class pqServer;
class pqSMTKWrapper;

/** \brief Register readers for all registered model import routines.
  *
  * This instance will construct server manager configuration input to register
  * a reader for each resource type.
  */
class SMTKPQCOMPONENTSEXT_EXPORT pqSMTKRegisterImportersBehavior : public QObject
{
  Q_OBJECT
  using Superclass = QObject;

public:
  static pqSMTKRegisterImportersBehavior* instance(QObject* parent = nullptr);
  ~pqSMTKRegisterImportersBehavior() override;

protected:
  pqSMTKRegisterImportersBehavior(QObject* parent = nullptr);

protected Q_SLOTS:
  void constructImporters(pqSMTKWrapper* rsrcMgr, pqServer* server);

private:
  Q_DISABLE_COPY(pqSMTKRegisterImportersBehavior);
};

#endif // smtk_extension_paraview_appcomponents_pqSMTKRegisterImportersBehavior_h
