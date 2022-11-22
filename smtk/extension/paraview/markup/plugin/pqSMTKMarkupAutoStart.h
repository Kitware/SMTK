//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_markup_pqSMTKMarkupAutoStart_h
#define smtk_extension_paraview_markup_pqSMTKMarkupAutoStart_h

#include <QObject>

class vtkSMProxy;

class pqSMTKMarkupAutoStart : public QObject
{
  Q_OBJECT
  using Superclass = QObject;

public:
  pqSMTKMarkupAutoStart(QObject* parent = nullptr);
  ~pqSMTKMarkupAutoStart() override;

  void startup();
  void shutdown();

private:
  Q_DISABLE_COPY(pqSMTKMarkupAutoStart);
};

#endif
