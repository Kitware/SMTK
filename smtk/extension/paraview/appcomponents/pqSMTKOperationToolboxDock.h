//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_pqSMTKOperationToolboxDock_h
#define smtk_extension_paraview_appcomponents_pqSMTKOperationToolboxDock_h

#include "smtk/extension/paraview/appcomponents/pqSMTKDock.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKOperationToolboxPanel.h"

class SMTKPQCOMPONENTSEXT_EXPORT pqSMTKOperationToolboxDock
  : public pqSMTKDock<pqSMTKOperationToolboxPanel>
{
  Q_OBJECT
public:
  pqSMTKOperationToolboxDock(QWidget* parent = nullptr)
    : pqSMTKDock<pqSMTKOperationToolboxPanel>("pqSMTKOperationToolboxDock", parent)
  {
  }
  ~pqSMTKOperationToolboxDock() override = default;
};
#endif // smtk_extension_paraview_appcomponents_pqSMTKOperationToolboxDock_h
