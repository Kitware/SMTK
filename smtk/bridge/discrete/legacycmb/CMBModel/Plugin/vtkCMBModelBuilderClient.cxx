//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "vtkCMBModelBuilderClient.h"

#include "vtkDiscreteModel.h"
#include "vtkObjectFactory.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMOperatorProxy.h"
#include "vtkSMProxyManager.h"
#include "vtkSMStringVectorProperty.h"
#include "vtkSmartPointer.h"
#include "vtkXMLModelReader.h"

#include <sstream>

vtkStandardNewMacro(vtkCMBModelBuilderClient);

vtkCMBModelBuilderClient::vtkCMBModelBuilderClient()
{
}

vtkCMBModelBuilderClient::~vtkCMBModelBuilderClient()
{
}

bool vtkCMBModelBuilderClient::Operate(vtkDiscreteModel* Model,
  vtkSMProxy* ServerModelProxy, vtkSMProxy* PolySourceProxy)
{
  if(!this->AbleToOperate(Model)|| PolySourceProxy == NULL ||
     ServerModelProxy == NULL)
    {
    return 0;
    }

  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMOperatorProxy* OperatorProxy = vtkSMOperatorProxy::SafeDownCast(
    manager->NewProxy("CMBModelGroup", "CMBModelBuilder"));
  if(!OperatorProxy)
    {
    vtkErrorMacro("Unable to create builder operator proxy.");
    return 0;
    }
  OperatorProxy->SetLocation(ServerModelProxy->GetLocation());

  OperatorProxy->Operate(Model, ServerModelProxy, PolySourceProxy);

  // check to see if the operation succeeded on the server
  vtkSMIntVectorProperty* OperateSucceeded =
    vtkSMIntVectorProperty::SafeDownCast(
      OperatorProxy->GetProperty("OperateSucceeded"));

  OperatorProxy->UpdatePropertyInformation();

  int Succeeded = OperateSucceeded->GetElement(0);
  OperatorProxy->Delete();
  OperatorProxy = 0;
  if(!Succeeded)
    {
    vtkErrorMacro("Server side operator failed.");
    return 0;
    }

  return vtkCMBModelBuilderClient::UpdateClientModel(Model, ServerModelProxy);
}


bool vtkCMBModelBuilderClient::AbleToOperate(vtkDiscreteModel* Model)
{
  if(!Model)
    {
    vtkErrorMacro("Passed in a null model.");
    return 0;
    }
  return 1;
}

//----------------------------------------------------------------------------
bool vtkCMBModelBuilderClient::UpdateClientModel(vtkDiscreteModel* ClientModel,
                                                vtkSMProxy* ServerModelProxy)
{
  // Model will get it's server side state and update it on the client.
  // Currently this is done with a StringVectorProperty but we may want
  // to subclass from vtkPVInformation and do
  // ProcessModule->GatherInformation();
  vtkSMStringVectorProperty* SerializedModel =
    vtkSMStringVectorProperty::SafeDownCast(
      ServerModelProxy->GetProperty("ModelSerialization"));

  if(!SerializedModel)
    {
    cerr << "Cannot get ModelSerialization property in wrapper proxy.\n";
    return 0;
    }

  ServerModelProxy->UpdatePropertyInformation(SerializedModel);
  const char* data = SerializedModel->GetElement(0);

  // Create an input stream to read the XML back
  std::istringstream istr(data);
  vtkSmartPointer<vtkXMLModelReader> reader =
    vtkSmartPointer<vtkXMLModelReader>::New();
  ClientModel->Reset();
  reader->SetModel(ClientModel);
  reader->Serialize(istr, "ConceptualModel");
  return 1;
}

void vtkCMBModelBuilderClient::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
