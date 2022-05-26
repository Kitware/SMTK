//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_paraview_appcomponents_pqSMTKAttributeDock_h
#define smtk_extension_paraview_appcomponents_pqSMTKAttributeDock_h

#include "smtk/extension/paraview/appcomponents/pqSMTKAttributePanel.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKDock.h"

class SMTKPQCOMPONENTSEXT_EXPORT pqSMTKAttributeDock : public pqSMTKDock<pqSMTKAttributePanel>
{
  Q_OBJECT
public:
  pqSMTKAttributeDock(QWidget* parent = nullptr)
    : pqSMTKDock<pqSMTKAttributePanel>("pqSMTKAttributeDock", parent)
  {
  }
  ~pqSMTKAttributeDock() override = default;
};
#endif // smtk_extension_paraview_appcomponents_pqSMTKAttributeDock_h
