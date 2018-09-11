//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkGeoTransformOperationClient.h"

#include "vtkDiscreteModel.h"
#include "vtkObjectFactory.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMOperationProxy.h"
#include "vtkSMProxyManager.h"
#include "vtkSMStringVectorProperty.h"
#include "vtkSmartPointer.h"
#include "vtkXMLModelReader.h"

#include <sstream>

vtkStandardNewMacro(vtkGeoTransformOperationClient);

vtkGeoTransformOperationClient::vtkGeoTransformOperationClient()
{
  this->ConvertFromLatLongToXYZ = false;
  this->OperationProxy = NULL;
}

vtkGeoTransformOperationClient::~vtkGeoTransformOperationClient()
{
  if (this->OperationProxy)
  {
    this->OperationProxy->Delete();
    this->OperationProxy = NULL;
  }
}

bool vtkGeoTransformOperationClient::Operate(vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy)
{
  if (!this->AbleToOperate(Model) || ServerModelProxy == NULL)
  {
    return 0;
  }

  if (!this->OperationProxy)
  {
    vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
    this->OperationProxy = vtkSMOperationProxy::SafeDownCast(
      manager->NewProxy("CMBModelGroup", "GeoTransformOperation"));
  }

  if (!this->OperationProxy)
  {
    vtkErrorMacro("Unable to create builder operator proxy.");
    return 0;
  }

  this->OperationProxy->SetLocation(ServerModelProxy->GetLocation());

  vtkSMIntVectorProperty* ivp = vtkSMIntVectorProperty::SafeDownCast(
    this->OperationProxy->GetProperty("ConvertFromLatLongToXYZ"));
  ivp->SetElement(0, this->ConvertFromLatLongToXYZ);
  this->OperationProxy->UpdateVTKObjects();

  this->OperationProxy->Operate(Model, ServerModelProxy);

  // check to see if the operation succeeded on the server
  vtkSMIntVectorProperty* OperateSucceeded =
    vtkSMIntVectorProperty::SafeDownCast(this->OperationProxy->GetProperty("OperateSucceeded"));

  this->OperationProxy->UpdatePropertyInformation();

  int Succeeded = OperateSucceeded->GetElement(0);
  if (!Succeeded)
  {
    vtkErrorMacro("Server side operator failed.");
    return 0;
  }

  return 1;
}

bool vtkGeoTransformOperationClient::AbleToOperate(vtkDiscreteModel* Model)
{
  if (!Model)
  {
    vtkErrorMacro("Passed in a null model.");
    return 0;
  }

  return 1;
}
void vtkGeoTransformOperationClient::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent
     << "Convert From Lat/Long to xyz: " << (this->ConvertFromLatLongToXYZ ? "On" : "Off");
}
