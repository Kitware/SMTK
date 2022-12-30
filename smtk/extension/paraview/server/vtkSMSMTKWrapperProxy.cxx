//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/server/vtkSMSMTKWrapperProxy.h"
#include "smtk/extension/paraview/server/vtkSMSMTKResourceRepresentationProxy.h"

#include "smtk/extension/paraview/server/vtkSMTKWrapper.h" // TODO: Remove the need for me

#include "vtkObjectFactory.h"

#include "vtkClientServerStream.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMRepresentationProxy.h"
#include "vtkSMSourceProxy.h"

using nlohmann::json;

vtkSMSMTKWrapperProxy* g_instance = nullptr;

vtkSMSMTKWrapperProxy* vtkSMSMTKWrapperProxy::New()
{
  if (!g_instance)
  {
    g_instance = new vtkSMSMTKWrapperProxy;
  }
  return g_instance;
}

vtkSMSMTKWrapperProxy* vtkSMSMTKWrapperProxy::Instance()
{
  return g_instance;
}

vtkSMSMTKWrapperProxy::vtkSMSMTKWrapperProxy() = default;

vtkSMSMTKWrapperProxy::~vtkSMSMTKWrapperProxy()
{
  if (this == g_instance)
  {
    g_instance = nullptr;
  }
}

void vtkSMSMTKWrapperProxy::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

smtk::resource::ManagerPtr vtkSMSMTKWrapperProxy::GetResourceManager() const
{
  // TODO: This should just "return this->Manager;" but we are getting things
  //       working in built-in mode first, so just directly fetch the version
  //       on the server and return it.
  auto* self = const_cast<vtkSMSMTKWrapperProxy*>(this); // VTK is not const-correct
  auto* wrapper = vtkSMTKWrapper::SafeDownCast(self->GetClientSideObject());
  return wrapper ? wrapper->GetResourceManager() : nullptr;
}

smtk::view::SelectionPtr vtkSMSMTKWrapperProxy::GetSelection() const
{
  // TODO: This should just "return this->Selection;" but we are getting things
  //       working in built-in mode first, so just directly fetch the version
  //       on the server and return it.
  auto* self = const_cast<vtkSMSMTKWrapperProxy*>(this); // VTK is not const-correct
  auto* wrapper = vtkSMTKWrapper::SafeDownCast(self->GetClientSideObject());
  return wrapper ? wrapper->GetSelection() : nullptr;
}

smtk::operation::ManagerPtr vtkSMSMTKWrapperProxy::GetOperationManager() const
{
  // TODO: This should just "return this->OperationManager;" but we are getting things
  //       working in built-in mode first, so just directly fetch the version
  //       on the server and return it.
  auto* self = const_cast<vtkSMSMTKWrapperProxy*>(this); // VTK is not const-correct
  auto* wrapper = vtkSMTKWrapper::SafeDownCast(self->GetClientSideObject());
  return wrapper ? wrapper->GetOperationManager() : nullptr;
}

smtk::project::ManagerPtr vtkSMSMTKWrapperProxy::GetProjectManager() const
{
  // TODO: This should just "return this->ProjectManager;" but we are getting things
  //       working in built-in mode first, so just directly fetch the version
  //       on the server and return it.
  auto* self = const_cast<vtkSMSMTKWrapperProxy*>(this); // VTK is not const-correct
  auto* wrapper = vtkSMTKWrapper::SafeDownCast(self->GetClientSideObject());
  return wrapper ? wrapper->GetProjectManager() : nullptr;
}

smtk::view::ManagerPtr vtkSMSMTKWrapperProxy::GetViewManager() const
{
  // TODO: This should just "return this->ViewManager;" but we are getting things
  //       working in built-in mode first, so just directly fetch the version
  //       on the server and return it.
  auto* self = const_cast<vtkSMSMTKWrapperProxy*>(this); // VTK is not const-correct
  auto* wrapper = vtkSMTKWrapper::SafeDownCast(self->GetClientSideObject());
  return wrapper ? wrapper->GetViewManager() : nullptr;
}

smtk::common::Managers::Ptr vtkSMSMTKWrapperProxy::GetManagersPtr() const
{
  // TODO: This should just "return this->Managers;" (i.e., the proxy should hold
  //       a "client adaptation" of the server's managers) and thus
  //       remove the need for direct access to the server data.
  auto* self = const_cast<vtkSMSMTKWrapperProxy*>(this); // VTK is not const-correct
  auto* wrapper = vtkSMTKWrapper::SafeDownCast(self->GetClientSideObject());
  return wrapper ? wrapper->GetManagersPtr() : nullptr;
}

void vtkSMSMTKWrapperProxy::SetSelectedPortProxy(vtkSMSourceProxy* pxy)
{
  vtkSMPropertyHelper(this, "SelectedPort").Set(pxy);
  this->UpdateVTKObjects();
}

void vtkSMSMTKWrapperProxy::SetSelectionObjProxy(vtkSMSourceProxy* pxy)
{
  vtkSMPropertyHelper(this, "SelectionObj").Set(pxy);
  this->UpdateVTKObjects();
}

void vtkSMSMTKWrapperProxy::FetchHardwareSelection()
{
  json request = {
    { "method", "fetch hw selection" },
    { "id", 1 },
  };
  json response = this->JSONRPCRequest(request);
  // std::cout << response.dump(2) << "\n";
}

void vtkSMSMTKWrapperProxy::AddResourceProxy(vtkSMSourceProxy* rsrc)
{
  vtkSMPropertyHelper(this, "ActiveResource").Set(rsrc);
  this->UpdateVTKObjects();

  json request = {
    { "method", "add resource filter" },
    { "id", 1 },
  };
  json response = this->JSONRPCRequest(request);
  // std::cout << response.dump(2) << "\n";
}

void vtkSMSMTKWrapperProxy::RemoveResourceProxy(vtkSMSourceProxy* rsrc)
{
  vtkSMPropertyHelper(this, "ActiveResource").Set(rsrc);
  this->UpdateVTKObjects();

  json request = {
    { "method", "remove resource filter" },
    { "id", 1 },
  };
  json response = this->JSONRPCRequest(request);
  // std::cout << response.dump(2) << "\n";
}

void vtkSMSMTKWrapperProxy::Send(const json& selnInfo)
{
  (void)selnInfo;
}

void vtkSMSMTKWrapperProxy::Recv(
  vtkSMSourceProxy* dataSource,
  vtkSMSourceProxy* selnSource,
  json& selnInfo)
{
  (void)dataSource;
  (void)selnSource;
  (void)selnInfo;
}

json vtkSMSMTKWrapperProxy::JSONRPCRequest(const json& request)
{
  return this->JSONRPCRequest(request.dump());
}

json vtkSMSMTKWrapperProxy::JSONRPCRequest(const std::string& request)
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

void vtkSMSMTKWrapperProxy::JSONRPCNotification(const json& note)
{
  this->JSONRPCNotification(note.dump());
}

void vtkSMSMTKWrapperProxy::JSONRPCNotification(const std::string& note)
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

void vtkSMSMTKWrapperProxy::SetRepresentation(vtkSMRepresentationProxy* pxy)
{
  auto* smtkProxy = vtkSMSMTKResourceRepresentationProxy::SafeDownCast(pxy);
  auto* repProxy = smtkProxy->GetResourceRepresentationSubProxy();

  vtkSMPropertyHelper(this, "Representation").Set(vtkSMProxy::SafeDownCast(repProxy));
  this->UpdateVTKObjects();
}

void vtkSMSMTKWrapperProxy::SetResourceForRepresentation(
  smtk::resource::ResourcePtr clientSideResource,
  vtkSMRepresentationProxy* pxy)
{
  this->SetRepresentation(pxy);
  json request = {
    { "method", "setup representation" },
    { "id", 1 },
    { "params",
      { { "resource",
          (clientSideResource ? clientSideResource->id() : smtk::common::UUID::null())
            .toString() } } }
  };

  json response = this->JSONRPCRequest(request);
  // std::cout << response.dump(2) << "\n"; // for debugging
}
