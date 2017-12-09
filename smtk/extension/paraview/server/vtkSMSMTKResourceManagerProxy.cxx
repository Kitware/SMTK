//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/server/vtkSMSMTKResourceManagerProxy.h"

#include "smtk/extension/paraview/server/vtkSMTKResourceManagerWrapper.h" // TODO: Remove the need for me

#include "vtkObjectFactory.h"

#include "vtkClientServerStream.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMSourceProxy.h"

using nlohmann::json;

vtkSMSMTKResourceManagerProxy* g_instance = nullptr;

vtkSMSMTKResourceManagerProxy* vtkSMSMTKResourceManagerProxy::New()
{
  if (!g_instance)
  {
    g_instance = new vtkSMSMTKResourceManagerProxy;
  }
  return g_instance;
}

vtkSMSMTKResourceManagerProxy* vtkSMSMTKResourceManagerProxy::Instance()
{
  return g_instance;
}

vtkSMSMTKResourceManagerProxy::vtkSMSMTKResourceManagerProxy()
{
}

vtkSMSMTKResourceManagerProxy::~vtkSMSMTKResourceManagerProxy()
{
  if (this == g_instance)
  {
    g_instance = nullptr;
  }
}

void vtkSMSMTKResourceManagerProxy::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

smtk::resource::ManagerPtr vtkSMSMTKResourceManagerProxy::GetManager() const
{
  // TODO: This should just "return this->Manager;" but we are getting things
  //       working in built-in mode first, so just directly fetch the version
  //       on the server and return it.
  auto self = const_cast<vtkSMSMTKResourceManagerProxy*>(this); // VTK is not const-correct
  auto wrapper = vtkSMTKResourceManagerWrapper::SafeDownCast(self->GetClientSideObject());
  return wrapper ? wrapper->GetManager() : nullptr;
}

smtk::view::SelectionPtr vtkSMSMTKResourceManagerProxy::GetSelection() const
{
  // TODO: This should just "return this->Selection;" but we are getting things
  //       working in built-in mode first, so just directly fetch the version
  //       on the server and return it.
  auto self = const_cast<vtkSMSMTKResourceManagerProxy*>(this); // VTK is not const-correct
  auto wrapper = vtkSMTKResourceManagerWrapper::SafeDownCast(self->GetClientSideObject());
  return wrapper ? wrapper->GetSelection() : nullptr;
}

smtk::operation::ManagerPtr vtkSMSMTKResourceManagerProxy::GetOperationManager() const
{
  // TODO: This should just "return this->OperationManager;" but we are getting things
  //       working in built-in mode first, so just directly fetch the version
  //       on the server and return it.
  auto self = const_cast<vtkSMSMTKResourceManagerProxy*>(this); // VTK is not const-correct
  auto wrapper = vtkSMTKResourceManagerWrapper::SafeDownCast(self->GetClientSideObject());
  return wrapper ? wrapper->GetOperationManager() : nullptr;
}

void vtkSMSMTKResourceManagerProxy::SetSelectedPortProxy(vtkSMSourceProxy* pxy)
{
  vtkSMPropertyHelper(this, "SelectedPort").Set(pxy);
  this->UpdateVTKObjects();
}

void vtkSMSMTKResourceManagerProxy::SetSelectionObjProxy(vtkSMSourceProxy* pxy)
{
  vtkSMPropertyHelper(this, "SelectionObj").Set(pxy);
  this->UpdateVTKObjects();
}

void vtkSMSMTKResourceManagerProxy::FetchHardwareSelection()
{
  json request = {
    { "method", "fetch hw selection" }, { "id", 1 },
  };
  json response = this->JSONRPCRequest(request);
  std::cout << response.dump(2) << "\n";
}

void vtkSMSMTKResourceManagerProxy::AddResourceProxy(vtkSMSourceProxy* rsrc)
{
  vtkSMPropertyHelper(this, "ActiveResource").Set(rsrc);
  this->UpdateVTKObjects();

  json request = {
    { "method", "add resource" }, { "id", 1 },
  };
  json response = this->JSONRPCRequest(request);
  std::cout << response.dump(2) << "\n";
}

void vtkSMSMTKResourceManagerProxy::RemoveResourceProxy(vtkSMSourceProxy* rsrc)
{
  vtkSMPropertyHelper(this, "ActiveResource").Set(rsrc);
  this->UpdateVTKObjects();

  json request = {
    { "method", "remove resource" }, { "id", 1 },
  };
  json response = this->JSONRPCRequest(request);
  std::cout << response.dump(2) << "\n";
}

void vtkSMSMTKResourceManagerProxy::Send(const json& selnInfo)
{
  (void)selnInfo;
}

void vtkSMSMTKResourceManagerProxy::Recv(
  vtkSMSourceProxy* dataSource, vtkSMSourceProxy* selnSource, json& selnInfo)
{
  (void)dataSource;
  (void)selnSource;
  (void)selnInfo;
}

json vtkSMSMTKResourceManagerProxy::JSONRPCRequest(const json& request)
{
  return this->JSONRPCRequest(request.dump());
}

json vtkSMSMTKResourceManagerProxy::JSONRPCRequest(const std::string& request)
{
  json result;
  if (request.empty())
  {
    return result;
  }

  this->JSONRPCNotification(request);

  // Now, unlike notifications, we expect a response.
  // Get the json response string and parse it:
  this->UpdatePropertyInformation();
  std::string response = vtkSMPropertyHelper(this, "JSONResponse").GetAsString();
  return json::parse(response);
}

void vtkSMSMTKResourceManagerProxy::JSONRPCNotification(const json& note)
{
  this->JSONRPCNotification(note.dump());
}

void vtkSMSMTKResourceManagerProxy::JSONRPCNotification(const std::string& note)
{
  if (note.empty())
  {
    return;
  }

  vtkSMPropertyHelper(this, "JSONRequest").Set(note.c_str());
  this->UpdateVTKObjects();
  vtkClientServerStream stream;
  stream << vtkClientServerStream::Invoke << VTKOBJECT(this) << "ProcessJSON"
         << vtkClientServerStream::End;
  this->ExecuteStream(stream);
}
