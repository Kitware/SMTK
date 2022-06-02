//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_pqSMTKOperationParameterDock_h
#define smtk_extension_paraview_appcomponents_pqSMTKOperationParameterDock_h

#include "smtk/extension/paraview/appcomponents/pqSMTKDock.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKOperationParameterPanel.h"

class SMTKPQCOMPONENTSEXT_EXPORT pqSMTKOperationParameterDock
  : public pqSMTKDock<pqSMTKOperationParameterPanel>
{
  Q_OBJECT
public:
  pqSMTKOperationParameterDock(QWidget* parent = nullptr)
    : pqSMTKDock<pqSMTKOperationParameterPanel>("pqSMTKOperationParameterDock", parent)
  {
  }
  ~pqSMTKOperationParameterDock() override = default;
};
#endif // smtk_extension_paraview_appcomponents_pqSMTKOperationParameterDock_h
