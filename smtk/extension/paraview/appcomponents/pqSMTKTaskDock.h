//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_pqSMTKTaskDock_h
#define smtk_extension_paraview_appcomponents_pqSMTKTaskDock_h

#include "smtk/extension/paraview/appcomponents/pqSMTKDock.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKTaskPanel.h"

class SMTKPQCOMPONENTSEXT_EXPORT pqSMTKTaskDock : public pqSMTKDock<pqSMTKTaskPanel>
{
  Q_OBJECT
public:
  pqSMTKTaskDock(QWidget* parent = nullptr)
    : pqSMTKDock<pqSMTKTaskPanel>("pqSMTKTaskDock", parent)
  {
  }
  ~pqSMTKTaskDock() override = default;
};
#endif // smtk_extension_paraview_appcomponents_pqSMTKTaskDock_h
