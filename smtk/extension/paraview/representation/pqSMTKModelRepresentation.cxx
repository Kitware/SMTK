//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/representation/pqSMTKModelRepresentation.h"

#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKResourceManager.h"

#include "smtk/resource/SelectionManager.h"

pqSMTKModelRepresentation::pqSMTKModelRepresentation(
  const QString& group, const QString& name, vtkSMProxy* repr, pqServer* server, QObject* parent)
  : Superclass(group, name, repr, server, parent)
{
  auto smtk = pqSMTKBehavior::instance();
  auto rsrcMgrPxy = smtk->resourceManagerForServer(server);
  if (rsrcMgrPxy)
  {
    rsrcMgrPxy->smtkSelection()->listenToSelectionEvents([this](const std::string src,
      smtk::resource::SelectionManagerPtr seln) { this->handleSMTKSelectionChange(src, seln); });
  }
}

pqSMTKModelRepresentation::~pqSMTKModelRepresentation()
{
}

void pqSMTKModelRepresentation::handleSMTKSelectionChange(
  const std::string& src, smtk::resource::SelectionManagerPtr seln)
{
  (void)src;
  (void)seln;
  this->renderViewEventually();
}
