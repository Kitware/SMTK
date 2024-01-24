//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_pqSMTKDiagramDock_h
#define smtk_extension_paraview_appcomponents_pqSMTKDiagramDock_h

#include "smtk/extension/paraview/appcomponents/pqSMTKDiagramPanel.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKDock.h"

class SMTKPQCOMPONENTSEXT_EXPORT pqSMTKDiagramDock : public pqSMTKDock<pqSMTKDiagramPanel>
{
  Q_OBJECT
public:
  pqSMTKDiagramDock(QWidget* parent = nullptr)
    : pqSMTKDock<pqSMTKDiagramPanel>("pqSMTKDiagramDock", parent)
  {
  }
  ~pqSMTKDiagramDock() override = default;
};
#endif // smtk_extension_paraview_appcomponents_pqSMTKDiagramDock_h
