//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_pqSMTKWidgetsAutoStart_h
#define smtk_extension_paraview_appcomponents_pqSMTKWidgetsAutoStart_h

#include <QObject>

class vtkSMProxy;

class pqSMTKWidgetsAutoStart : public QObject
{
  Q_OBJECT
  using Superclass = QObject;

public:
  pqSMTKWidgetsAutoStart(QObject* parent = nullptr);
  ~pqSMTKWidgetsAutoStart() override;

  void startup();
  void shutdown();

private:
  Q_DISABLE_COPY(pqSMTKWidgetsAutoStart);
};

#endif
