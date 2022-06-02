//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_pqSMTKOperationDock_h
#define smtk_extension_paraview_appcomponents_pqSMTKOperationDock_h

#include "smtk/extension/paraview/appcomponents/pqSMTKDock.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKOperationPanel.h"

class SMTKPQCOMPONENTSEXT_EXPORT pqSMTKOperationDock : public pqSMTKDock<pqSMTKOperationPanel>
{
  Q_OBJECT
public:
  pqSMTKOperationDock(QWidget* parent = nullptr)
    : pqSMTKDock<pqSMTKOperationPanel>("pqSMTKOperationDock", parent)
  {
  }
  ~pqSMTKOperationDock() override = default;
};
#endif // smtk_extension_paraview_appcomponents_pqSMTKOperationDock_h
