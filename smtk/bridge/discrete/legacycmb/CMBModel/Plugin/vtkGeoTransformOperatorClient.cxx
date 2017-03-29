//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "vtkGeoTransformOperatorClient.h"

#include "vtkDiscreteModel.h"
#include "vtkObjectFactory.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMOperatorProxy.h"
#include "vtkSMProxyManager.h"
#include "vtkSMStringVectorProperty.h"
#include "vtkSmartPointer.h"
#include "vtkXMLModelReader.h"

#include <sstream>

vtkStandardNewMacro(vtkGeoTransformOperatorClient);

vtkGeoTransformOperatorClient::vtkGeoTransformOperatorClient()
{
  this->ConvertFromLatLongToXYZ = false;
  this->OperatorProxy = NULL;
}

vtkGeoTransformOperatorClient::~vtkGeoTransformOperatorClient()
{
  if(this->OperatorProxy)
    {
    this->OperatorProxy->Delete();
    this->OperatorProxy = NULL;
    }
}

bool vtkGeoTransformOperatorClient::Operate(vtkDiscreteModel* Model,
  vtkSMProxy* ServerModelProxy)
{
  if(!this->AbleToOperate(Model)||
     ServerModelProxy == NULL)
    {
    return 0;
    }

  if(!this->OperatorProxy)
    {
    vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
    this->OperatorProxy = vtkSMOperatorProxy::SafeDownCast(
      manager->NewProxy("CMBModelGroup", "GeoTransformOperator"));
    }

  if(!this->OperatorProxy)
    {
    vtkErrorMacro("Unable to create builder operator proxy.");
    return 0;
    }

  this->OperatorProxy->SetLocation(ServerModelProxy->GetLocation());

  vtkSMIntVectorProperty* ivp = vtkSMIntVectorProperty::SafeDownCast(
    this->OperatorProxy->GetProperty("ConvertFromLatLongToXYZ"));
  ivp->SetElement(0, this->ConvertFromLatLongToXYZ);
  this->OperatorProxy->UpdateVTKObjects();

  this->OperatorProxy->Operate(Model, ServerModelProxy);

  // check to see if the operation succeeded on the server
  vtkSMIntVectorProperty* OperateSucceeded =
    vtkSMIntVectorProperty::SafeDownCast(
      this->OperatorProxy->GetProperty("OperateSucceeded"));

  this->OperatorProxy->UpdatePropertyInformation();

  int Succeeded = OperateSucceeded->GetElement(0);
  if(!Succeeded)
    {
    vtkErrorMacro("Server side operator failed.");
    return 0;
    }

  return 1;
}


bool vtkGeoTransformOperatorClient::AbleToOperate(vtkDiscreteModel* Model)
{
  if(!Model)
    {
    vtkErrorMacro("Passed in a null model.");
    return 0;
    }

  return 1;
}
void vtkGeoTransformOperatorClient::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "Convert From Lat/Long to xyz: " <<
    (this->ConvertFromLatLongToXYZ ? "On" : "Off");

}
