//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "vtkCMBModelOmicronMeshInputWriterClient.h"

#include "vtkDiscreteModel.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"
#include "vtkSMDoubleVectorProperty.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMOperatorProxy.h"
#include "vtkSMProxyManager.h"
#include "vtkSMStringVectorProperty.h"

vtkStandardNewMacro(vtkCMBModelOmicronMeshInputWriterClient);

//----------------------------------------------------------------------------
vtkCMBModelOmicronMeshInputWriterClient::vtkCMBModelOmicronMeshInputWriterClient()
{
  this->FileName = 0;
  this->TetGenOptions = 0;
  this->GeometryFileName = 0;
  this->VolumeConstraint = 0.001;
}

//----------------------------------------------------------------------------
vtkCMBModelOmicronMeshInputWriterClient:: ~vtkCMBModelOmicronMeshInputWriterClient()
{
  this->SetFileName(0);
  this->SetGeometryFileName(0);
  this->SetTetGenOptions(0);
}

//----------------------------------------------------------------------------
bool vtkCMBModelOmicronMeshInputWriterClient::Operate(vtkDiscreteModel* model,
                                                      vtkSMProxy* serverModelProxy)
{
  if(!this->GetFileName())
    {
    vtkWarningMacro("Must set file name.");
    return 0;
    }

  if(!model)
    {
    vtkErrorMacro("Passed in a null model.");
    return 0;
    }

  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMOperatorProxy* operatorProxy = vtkSMOperatorProxy::SafeDownCast(
    manager->NewProxy("CMBModelGroup", "CMBModelOmicronMeshInputWriter"));
  if(!operatorProxy)
    {
    vtkErrorMacro("Unable to create operator proxy.");
    return 0;
    }
  operatorProxy->SetLocation(serverModelProxy->GetLocation());

  vtkSMDoubleVectorProperty* volumeContraintProperty =
    vtkSMDoubleVectorProperty::SafeDownCast(
      operatorProxy->GetProperty("VolumeConstraint"));
  volumeContraintProperty->SetElement(0, this->VolumeConstraint);

  vtkSMStringVectorProperty* strProperty =
    vtkSMStringVectorProperty::SafeDownCast(
      operatorProxy->GetProperty("FileName") );
  strProperty->SetElement(0, this->FileName);
  strProperty->SetElementType(0, vtkSMStringVectorProperty::STRING);

  vtkSMStringVectorProperty* strProperty2 =
    vtkSMStringVectorProperty::SafeDownCast(
      operatorProxy->GetProperty("GeometryFileName") );
  strProperty2->SetElement(0, this->GeometryFileName);
  strProperty2->SetElementType(0, vtkSMStringVectorProperty::STRING);

  vtkSMStringVectorProperty* strProperty3 =
    vtkSMStringVectorProperty::SafeDownCast(
      operatorProxy->GetProperty("TetGenOptions") );
  strProperty3->SetElement(0, this->TetGenOptions);
  strProperty3->SetElementType(0, vtkSMStringVectorProperty::STRING);

  operatorProxy->Operate(model, serverModelProxy);

  // check to see if the operation succeeded on the server
  vtkSMIntVectorProperty* operateSucceeded =
    vtkSMIntVectorProperty::SafeDownCast(
      operatorProxy->GetProperty("OperateSucceeded"));

  operatorProxy->UpdatePropertyInformation();

  int succeeded = operateSucceeded->GetElement(0);
  operatorProxy->Delete();
  if(!succeeded)
    {
    vtkErrorMacro("Server side operator failed.");
    return 0;
    }

  return 1;
}

//----------------------------------------------------------------------------
void vtkCMBModelOmicronMeshInputWriterClient::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "FileName: " << this->FileName << endl;
  os << indent << "Geometry FileName: " << this->GeometryFileName << endl;
  os << indent << "VolumeConstraint: " << this->VolumeConstraint << endl;
}
