//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/pqSMTKModelRepresentation.h"
#include "smtk/extension/paraview/server/vtkSMSMTKModelRepresentationProxy.h"
#include "smtk/extension/paraview/server/vtkSMSMTKWrapperProxy.h"
#include "smtk/extension/paraview/server/vtkSMTKModelRepresentation.h" // TODO Remove the need for me.

#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKResource.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKWrapper.h"

#include "smtk/extension/paraview/server/vtkSMTKSettings.h"

#include "smtk/model/Entity.h"
#include "smtk/view/Selection.h"

#include "pqCoreUtilities.h"

#include "vtkSMPropertyHelper.h"

#include "vtkPVCompositeRepresentation.h"

#include "vtkCommand.h"

pqSMTKModelRepresentation::pqSMTKModelRepresentation(
  const QString& group, const QString& name, vtkSMProxy* repr, pqServer* server, QObject* parent)
  : Superclass(group, name, repr, server, parent)
  , m_selnObserver(-1)
{
  auto smtk = pqSMTKBehavior::instance();
  auto rsrcMgrPxy = smtk->resourceManagerForServer(server);
  if (rsrcMgrPxy)
  {
    auto seln = rsrcMgrPxy->smtkSelection();
    m_seln = seln;
    m_selnObserver = seln->observe([this](const std::string src, smtk::view::SelectionPtr oseln) {
      this->handleSMTKSelectionChange(src, oseln);
    });
  }

  // Subscribe to settings updates...
  auto smtkSettings = vtkSMTKSettings::GetInstance();
  pqCoreUtilities::connect(smtkSettings, vtkCommand::ModifiedEvent, this, SLOT(updateSettings()));
  // ... and initialize from current settings:
  this->updateSettings();
}

pqSMTKModelRepresentation::~pqSMTKModelRepresentation()
{
  // Avoid getting observations after we are dead.
  smtk::view::SelectionPtr seln = m_seln.lock();
  if (seln)
  {
    seln->unobserve(m_selnObserver);
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

void pqSMTKModelRepresentation::onInputChanged()
{
  pqPipelineRepresentation::onInputChanged();

  auto smtk = pqSMTKBehavior::instance();
  auto rsrcMgrPxy = smtk->resourceManagerForServer(this->getServer());
  if (rsrcMgrPxy)
  {
    // The representation needs some entity information (for color-by-volume for
    // instance), so here we set the manager resource.
    auto input = qobject_cast<pqSMTKResource*>(this->getInput());
    if (input)
    {
      rsrcMgrPxy->smtkProxy()->SetResourceForRepresentation(
        std::static_pointer_cast<smtk::resource::Resource>(input->getResource()),
        vtkSMSMTKModelRepresentationProxy::SafeDownCast(this->getProxy()));
    }
  }
}

bool pqSMTKModelRepresentation::setVisibility(smtk::resource::ComponentPtr comp, bool visible)
{
  auto pxy = this->getProxy();
  auto mpr = pxy->GetClientSideObject(); // TODO: Remove the need for me.
  auto cmp = vtkPVCompositeRepresentation::SafeDownCast(mpr);
  auto spx =
    cmp ? vtkSMTKModelRepresentation::SafeDownCast(cmp->GetActiveRepresentation()) : nullptr;
  if (spx)
  {
    if (spx->SetEntityVisibility(comp, visible))
    {
      emit componentVisibilityChanged(comp, visible);
      return true;
    }
  }
  return false;
}

void pqSMTKModelRepresentation::updateSettings()
{
  auto settings = vtkSMTKSettings::GetInstance();
  int selectionStyle = settings->GetSelectionRenderStyle();
  vtkSMPropertyHelper(this->getProxy(), "SelectionRenderStyle").Set(selectionStyle);
  this->getProxy()->UpdateVTKObjects();
  this->renderViewEventually();
}
