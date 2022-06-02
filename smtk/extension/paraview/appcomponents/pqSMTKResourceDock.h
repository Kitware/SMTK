//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_pqSMTKResourceDock_h
#define smtk_extension_paraview_appcomponents_pqSMTKResourceDock_h

#include "smtk/extension/paraview/appcomponents/pqSMTKDock.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKResourcePanel.h"

class SMTKPQCOMPONENTSEXT_EXPORT pqSMTKResourceDock : public pqSMTKDock<pqSMTKResourcePanel>
{
  Q_OBJECT
public:
  pqSMTKResourceDock(QWidget* parent = nullptr)
    : pqSMTKDock<pqSMTKResourcePanel>("pqSMTKResourceDock", parent)
  {
  }
  ~pqSMTKResourceDock() override = default;
};
#endif // smtk_extension_paraview_appcomponents_pqSMTKResourceDock_h
