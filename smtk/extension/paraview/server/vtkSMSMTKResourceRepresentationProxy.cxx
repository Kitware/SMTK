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
