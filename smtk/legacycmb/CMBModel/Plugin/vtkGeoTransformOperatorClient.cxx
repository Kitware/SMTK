/*=========================================================================

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/

#include "vtkGeoTransformOperatorClient.h"

#include "vtkDiscreteModel.h"
#include "vtkXMLModelReader.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMOperatorProxy.h"
#include "vtkSMProxyManager.h"
#include "vtkSMStringVectorProperty.h"

#include <vtksys/ios/sstream>

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
