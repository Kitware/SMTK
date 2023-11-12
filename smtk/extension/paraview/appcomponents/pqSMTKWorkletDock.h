//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_pqSMTKWorkletDock_h
#define smtk_extension_paraview_appcomponents_pqSMTKWorkletDock_h

#include "smtk/extension/paraview/appcomponents/pqSMTKDock.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKWorkletToolboxPanel.h"

class SMTKPQCOMPONENTSEXT_EXPORT pqSMTKWorkletDock : public pqSMTKDock<pqSMTKWorkletToolboxPanel>
{
  Q_OBJECT
public:
  pqSMTKWorkletDock(QWidget* parent = nullptr)
    : pqSMTKDock<pqSMTKWorkletToolboxPanel>("pqSMTKWorkletDock", parent)
  {
  }
  ~pqSMTKWorkletDock() override = default;
};
#endif // smtk_extension_paraview_appcomponents_pqSMTKWorkletDock_h
