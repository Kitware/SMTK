//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkSMSMTKModelRepresentationProxy.h"
#include "vtkSMTKModelRepresentation.h"

#include "vtkClientServerStream.h"
#include "vtkObjectFactory.h"
#include "vtkPVExtractSelection.h"
#include "vtkPVXMLElement.h"
#include "vtkSMChartSeriesListDomain.h"
#include "vtkSMDomain.h"
#include "vtkSMProperty.h"
#include "vtkSMPropertyHelper.h"

vtkStandardNewMacro(vtkSMSMTKModelRepresentationProxy);

vtkSMSMTKModelRepresentationProxy::vtkSMSMTKModelRepresentationProxy() = default;

vtkSMSMTKModelRepresentationProxy::~vtkSMSMTKModelRepresentationProxy() = default;

vtkSMTKModelRepresentation* vtkSMSMTKModelRepresentationProxy::GetRepresentation()
{
  this->CreateVTKObjects();
  return vtkSMTKModelRepresentation::SafeDownCast(this->GetClientSideObject());
}

void vtkSMSMTKModelRepresentationProxy::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

void vtkSMSMTKModelRepresentationProxy::SetPropertyModifiedFlag(const char* name, int flag)
{
  if (!name)
  {
    return;
  }

  if (strcmp(name, "Input") == 0)
  {
    vtkSMPropertyHelper helper(this, name);
    vtkSMSourceProxy* input = vtkSMSourceProxy::SafeDownCast(helper.GetAsProxy(0));
    if (input)
    {
      auto source = vtkSMSourceProxy::SafeDownCast(input->GetTrueParentProxy());
      if (source)
      {
        vtkSMPropertyHelper(this, "GlyphPrototypes", true).Set(source, 1);
        vtkSMPropertyHelper(this, "GlyphPoints", true).Set(source, 2);
        this->UpdateVTKObjects();
      }
    }
  }

  this->Superclass::SetPropertyModifiedFlag(name, flag);
}
