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

#include "vtkCmbMeshGridRepresentationClient.h"

#include "vtkDiscreteModel.h"
#include "vtkCMBXMLModelReader.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMOperatorProxy.h"
#include "vtkSMProxyManager.h"
#include "vtkSMProxyProperty.h"
#include "vtkSMStringVectorProperty.h"
#include "vtkSMPropertyHelper.h"

#include <vtksys/ios/sstream>

vtkStandardNewMacro(vtkCmbMeshGridRepresentationClient);
vtkCxxRevisionMacro(vtkCmbMeshGridRepresentationClient, "");
vtkCxxSetObjectMacro(vtkCmbMeshGridRepresentationClient, MeshRepresentationSource, vtkSMProxy);

vtkCmbMeshGridRepresentationClient::vtkCmbMeshGridRepresentationClient()
{
  this->MeshIsAnalysisGrid = 0;
  this->GridFileName = 0;
  this->MeshRepresentationSource = NULL;
}

vtkCmbMeshGridRepresentationClient::~vtkCmbMeshGridRepresentationClient()
{
  this->SetGridFileName(0);
  this->SetMeshRepresentationSource(NULL);
}

bool vtkCmbMeshGridRepresentationClient::Operate(vtkDiscreteModel *model,
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

void vtkCmbMeshGridRepresentationClient::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
