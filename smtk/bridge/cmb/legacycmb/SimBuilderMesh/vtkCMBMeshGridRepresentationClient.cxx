//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "vtkCMBMeshGridRepresentationClient.h"

#include "vtkDiscreteModel.h"
#include "vtkXMLModelReader.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMOperatorProxy.h"
#include "vtkSMProxyManager.h"
#include "vtkSMProxyProperty.h"
#include "vtkSMStringVectorProperty.h"
#include "vtkSMPropertyHelper.h"

#include <vtksys/ios/sstream>

vtkStandardNewMacro(vtkCMBMeshGridRepresentationClient);
vtkCxxSetObjectMacro(vtkCMBMeshGridRepresentationClient, MeshRepresentationSource, vtkSMProxy);

vtkCMBMeshGridRepresentationClient::vtkCMBMeshGridRepresentationClient()
{
  this->MeshIsAnalysisGrid = 0;
  this->GridFileName = 0;
  this->MeshRepresentationSource = NULL;
}

vtkCMBMeshGridRepresentationClient::~vtkCMBMeshGridRepresentationClient()
{
  this->SetGridFileName(0);
  this->SetMeshRepresentationSource(NULL);
}

bool vtkCMBMeshGridRepresentationClient::Operate(vtkDiscreteModel *model,
  vtkSMProxy *serverMeshProxy)
{

  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMOperatorProxy* operatorProxy = vtkSMOperatorProxy::SafeDownCast(
    manager->NewProxy("CMBSimBuilderMeshGroup", "CmbMeshGridRepresentationOperator"));
  if(!operatorProxy)
    {
    vtkErrorMacro("Unable to create operator proxy.");
    return 0;
    }
  operatorProxy->SetLocation(serverMeshProxy->GetLocation());
  vtkSMPropertyHelper(operatorProxy, "AnalysisMesh").Set(this->MeshIsAnalysisGrid);
  vtkSMPropertyHelper(operatorProxy,"GridFileName").Set(this->GridFileName);
  if(this->MeshRepresentationSource)
    {
    vtkSMProxyProperty* pp = vtkSMProxyProperty::SafeDownCast(
      operatorProxy->GetProperty("MeshRepresentationInput"));
    pp->RemoveAllProxies();
    pp->AddProxy(this->MeshRepresentationSource);
    }

  operatorProxy->Operate(model, serverMeshProxy);

  // check to see if the operation succeeded on the server
  vtkSMIntVectorProperty* operateSucceeded =
    vtkSMIntVectorProperty::SafeDownCast(
      operatorProxy->GetProperty("OperateSucceeded"));

  operatorProxy->UpdatePropertyInformation();

  int succeeded = operateSucceeded->GetElement(0);
  operatorProxy->Delete();
  operatorProxy = 0;
  if(!succeeded)
    {
    vtkErrorMacro("Server side operator failed.");
    return 0;
    }

  return 1;
}

void vtkCMBMeshGridRepresentationClient::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
