//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/pqSMTKDock.h"

pqSMTKDockBase::pqSMTKDockBase(QWidget* parent)
  : Superclass("baseDock", parent)
{
}

pqSMTKDockBase::~pqSMTKDockBase() = default;
