//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBSMModelProxy.h"

#include "vtkDiscreteModel.h"
#include "vtkObjectFactory.h"
#include "vtkProcessModule.h"

vtkStandardNewMacro(vtkCMBSMModelProxy);
//----------------------------------------------------------------------------
vtkCMBSMModelProxy::vtkCMBSMModelProxy()
{
  this->SetLocation(vtkProcessModule::CLIENT_AND_SERVERS);
}

//----------------------------------------------------------------------------
vtkCMBSMModelProxy::~vtkCMBSMModelProxy()
{
}

//----------------------------------------------------------------------------
void vtkCMBSMModelProxy::Refresh()
{
  vtkDiscreteModel* model = vtkDiscreteModel::SafeDownCast(this->GetClientSideObject());

}

//----------------------------------------------------------------------------
void vtkCMBSMModelProxy::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}


