//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/server/vtkSMSMTKResourceRepresentationProxy.h"
#include "smtk/extension/paraview/server/vtkSMTKResourceRepresentation.h"

#include "vtkClientServerStream.h"
#include "vtkCompositeRepresentation.h"
#include "vtkObjectFactory.h"
#include "vtkPVExtractSelection.h"
#include "vtkPVXMLElement.h"
#include "vtkSMChartSeriesListDomain.h"
#include "vtkSMDomain.h"
#include "vtkSMProperty.h"
#include "vtkSMPropertyHelper.h"

vtkStandardNewMacro(vtkSMSMTKResourceRepresentationProxy);

vtkSMSMTKResourceRepresentationProxy::vtkSMSMTKResourceRepresentationProxy() = default;

vtkSMSMTKResourceRepresentationProxy::~vtkSMSMTKResourceRepresentationProxy() = default;

void vtkSMSMTKResourceRepresentationProxy::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

vtkSMProxy* vtkSMSMTKResourceRepresentationProxy::GetResourceRepresentationSubProxy()
{
  return this->GetSubProxy("SMTKResourceRepresentation");
}

void vtkSMSMTKResourceRepresentationProxy::GetComponentVisibilities(
  std::map<smtk::common::UUID, int>& visibilities)
{
  // This method should really use client-server JSON RPC (in vtkSMTKWrapperProxy)
  // to send/receive the visibility map instead of "peeking under the covers" and
  // asking the server-side object directly.
  auto* mpr = this->GetClientSideObject(); // TODO: Remove the need for me.
  auto* cmp = vtkCompositeRepresentation::SafeDownCast(mpr);
  auto* spx =
    cmp ? vtkSMTKResourceRepresentation::SafeDownCast(cmp->GetActiveRepresentation()) : nullptr;
  if (spx)
  {
    spx->GetEntityVisibilities(visibilities);
  }
}
