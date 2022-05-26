//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_project_pqSMTKProjectDock_h
#define smtk_extension_paraview_project_pqSMTKProjectDock_h

#include "smtk/extension/paraview/appcomponents/pqSMTKDock.h"
#include "smtk/extension/paraview/project/pqSMTKProjectPanel.h"

class SMTKPQPROJECTEXT_EXPORT pqSMTKProjectDock : public pqSMTKDock<pqSMTKProjectPanel>
{
  Q_OBJECT
public:
  pqSMTKProjectDock(QWidget* parent = nullptr)
    : pqSMTKDock<pqSMTKProjectPanel>("pqSMTKProjectDock", parent)
  {
  }
  ~pqSMTKProjectDock() override = default;
};
#endif // smtk_extension_paraview_project_pqSMTKProjectDock_h
