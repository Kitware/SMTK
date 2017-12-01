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
#include "smtk/extension/paraview/representation/vtkSMSMTKModelRepresentationProxy.h"

#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKResourceManager.h"

#include "smtk/view/Selection.h"

pqSMTKModelRepresentation::pqSMTKModelRepresentation(
  const QString& group, const QString& name, vtkSMProxy* repr, pqServer* server, QObject* parent)
  : Superclass(group, name, repr, server, parent)
  , m_selnObserver(-1)
{
  auto smtk = pqSMTKBehavior::instance();
  auto rsrcMgrPxy = smtk->resourceManagerForServer(server);
  if (rsrcMgrPxy)
  {
    m_selnObserver = rsrcMgrPxy->smtkSelection()->observe([this](const std::string src,
      smtk::view::SelectionPtr seln) { this->handleSMTKSelectionChange(src, seln); });
  }
}

pqSMTKModelRepresentation::~pqSMTKModelRepresentation()
{
  // Avoid getting observations after we are dead.
  if (m_selnObserver >= 0)
  {
    auto smtk = pqSMTKBehavior::instance();
    auto rsrcMgrPxy = smtk->resourceManagerForServer(this->getServer());
    if (rsrcMgrPxy)
    {
      rsrcMgrPxy->smtkSelection()->unobserve(m_selnObserver);
    }
  }
}

void pqSMTKModelRepresentation::initialize()
{
  auto proxy = vtkSMSMTKModelRepresentationProxy::SafeDownCast(this->getProxy());
  if (proxy)
    proxy->ConnectAdditionalPorts();

  pqPipelineRepresentation::initialize();
}

void pqSMTKModelRepresentation::handleSMTKSelectionChange(
  const std::string& src, smtk::view::SelectionPtr seln)
{
  (void)src;
  (void)seln;
  this->renderViewEventually();
}
