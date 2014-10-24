//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "vtkCMBModelBuilder2DClient.h"

#include "vtkClientServerStream.h"
#include "vtkDiscreteModel.h"
#include "vtkXMLModelReader.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMOperatorProxy.h"
#include "vtkSMProxyManager.h"
#include "vtkSMSession.h"
#include "vtkSMStringVectorProperty.h"

#include <vtksys/ios/sstream>

vtkStandardNewMacro(vtkCMBModelBuilder2DClient);

//----------------------------------------------------------------------------
vtkCMBModelBuilder2DClient::vtkCMBModelBuilder2DClient()
{
}

//----------------------------------------------------------------------------
vtkCMBModelBuilder2DClient::~vtkCMBModelBuilder2DClient()
{
}

//----------------------------------------------------------------------------
bool vtkCMBModelBuilder2DClient::Operate(vtkDiscreteModel* Model,
  vtkSMProxy* ModelWrapper, vtkSMProxy* PolySourceProxy, int cleanVerts)
{
  if(!this->AbleToOperate(Model)|| PolySourceProxy == NULL ||
     ModelWrapper == NULL)
    {
    return 0;
    }

  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMOperatorProxy* operatorProxy = vtkSMOperatorProxy::SafeDownCast(
    manager->NewProxy("CMBModelGroup", "GenerateSimpleModelOperator"));
  if(!operatorProxy)
    {
    vtkErrorMacro("Unable to create builder operator proxy.");
    return 0;
    }
  operatorProxy->SetLocation(ModelWrapper->GetLocation());

  operatorProxy->UpdateVTKObjects();
  vtkClientServerStream stream;
  stream  << vtkClientServerStream::Invoke
          << VTKOBJECT(operatorProxy) << "Operate"
          << VTKOBJECT(ModelWrapper)
          << VTKOBJECT(PolySourceProxy)
          << cleanVerts
          << vtkClientServerStream::End;
  ModelWrapper->GetSession()->ExecuteStream(ModelWrapper->GetLocation(), stream);

  // check to see if the operation succeeded on the server
  vtkSMIntVectorProperty* operateSucceeded =
    vtkSMIntVectorProperty::SafeDownCast(
      operatorProxy->GetProperty("OperateSucceeded"));
  operatorProxy->UpdatePropertyInformation();

  int succeeded = operateSucceeded->GetElement(0);
  operatorProxy->Delete();
  operatorProxy = NULL;
  if(!succeeded)
    {
    vtkErrorMacro("Server side operator failed.");
    return 0;
    }
  return vtkCMBModelBuilder2DClient::UpdateClientModel(Model, ModelWrapper);
}

//----------------------------------------------------------------------------
bool vtkCMBModelBuilder2DClient::AbleToOperate(vtkDiscreteModel* Model)
{
  if(!Model)
    {
    vtkErrorMacro("Passed in a null model.");
    return 0;
    }
  return 1;
}

//----------------------------------------------------------------------------
bool vtkCMBModelBuilder2DClient::UpdateClientModel(vtkDiscreteModel* ClientModel,
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
  vtksys_ios::istringstream istr(data);
  vtkSmartPointer<vtkXMLModelReader> reader =
    vtkSmartPointer<vtkXMLModelReader>::New();
  ClientModel->Reset();
  reader->SetModel(ClientModel);
  reader->Serialize(istr, "ConceptualModel");
  return 1;
}

//----------------------------------------------------------------------------
void vtkCMBModelBuilder2DClient::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
