//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_pqSMTKGroupingAutoStart_h
#define smtk_extension_paraview_appcomponents_pqSMTKGroupingAutoStart_h

#include "smtk/extension/paraview/appcomponents/smtkPQComponentsExtModule.h"

#include "smtk/extension/paraview/appcomponents/pqQtKeywordWrapping.h"

#include <QObject>

class pqSMTKWrapper;
class pqServer;
class vtkSMProxy;

class SMTKPQCOMPONENTSEXT_EXPORT pqSMTKGroupingAutoStart : public QObject
{
  Q_OBJECT
  using Superclass = QObject;

  static vtkSMProxy* resourceManager();

public:
  pqSMTKGroupingAutoStart(QObject* parent = nullptr);
  ~pqSMTKGroupingAutoStart() override;

  void startup();
  void shutdown();

protected:
  class pqInternal;
  pqInternal* m_p;

private:
  Q_DISABLE_COPY(pqSMTKGroupingAutoStart);
};

#endif
