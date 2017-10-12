//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/server/vtkSMTKResourceManagerWrapper.h"

#include "smtk/io/jsonUUID.h"

#include "nlohmann/json.hpp"

#define JSONRPC_METHOD_NOT_FOUND_CODE -32601
#define JSONRPC_METHOD_NOT_FOUND_MESSAGE "Method not found"

using namespace nlohmann;

static vtkSMTKResourceManagerWrapper* s_instance = nullptr;

static void destroyInstance()
{
  if (s_instance)
  {
    s_instance->Delete();
    s_instance = nullptr;
  }
}

vtkSMTKResourceManagerWrapper* vtkSMTKResourceManagerWrapper::New()
{
  if (!s_instance)
  {
    s_instance = new vtkSMTKResourceManagerWrapper;
    atexit(destroyInstance);
  }
  return s_instance;
}

vtkSMTKResourceManagerWrapper::vtkSMTKResourceManagerWrapper()
  : JSONRequest(nullptr)
  , JSONResponse(nullptr)
{
}

vtkSMTKResourceManagerWrapper::~vtkSMTKResourceManagerWrapper()
{
  this->SetJSONRequest(nullptr);
  this->SetJSONResponse(nullptr);
}

void vtkSMTKResourceManagerWrapper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  indent = indent.GetNextIndent();
  os << indent << "JSONRequest: " << (this->JSONRequest ? this->JSONRequest : "null") << "\n";
  os << indent << "JSONResponse: " << (this->JSONResponse ? this->JSONResponse : "null") << "\n";
  os << indent << "SelectionListeners: " << this->SelectionListeners.size() << "\n";
}

int vtkSMTKResourceManagerWrapper::Listen(SelectionChangedFunction fn, bool sendImmediately)
{
  if (!fn)
  {
    return -1;
  }
  int handle = this->SelectionListeners.empty() ? 0 : this->SelectionListeners.rbegin()->first + 1;
  this->SelectionListeners[handle] = fn;
  if (sendImmediately)
  {
    fn(this->Selection, this->Selection, UUIDs(), "");
  }
  return handle;
}

void vtkSMTKResourceManagerWrapper::ProcessJSON()
{
  if (!this->JSONRequest || !this->JSONRequest[0])
  {
    return;
  }

  json j = json::parse(this->JSONRequest);
  if (j.is_null() || j.find("method") == j.end())
  {
    return;
  }
  json response = { { "jsonRPC", "2.0" }, { "id", j["id"].get<int>() } };

  auto method = j["method"].get<std::string>();
  if (method == "set selection")
  {
    json params = j["params"];
    UUIDs selnAdd;
    UUIDs selnDel;
    // Either the entire selection will be sent or +/- changes, but never both:
    if (params.find("selection") != params.end())
    {                            // Entire set sent. Deserialize and compute deltas.
      selnDel = this->Selection; // Copy the current selection into the list of deleted entries.
      this->Selection =
        params["selection"].get<UUIDs>(); // Copy the new selection into our storage.
      for (auto uid : this->Selection)
      {
        auto dit = selnDel.find(uid);
        if (dit == selnDel.end())
        { // We didn't have it before: it's new.
          selnAdd.insert(uid);
        }
        else
        { // We had it before and it's still here: it's not being removed. So erase it from selnDel:
          selnDel.erase(dit);
        }
      }
    }
    else
    { // Deltas sent. Update local storage to match.
      selnAdd = params["added"].get<UUIDs>();
      selnDel = params["removed"].get<UUIDs>();
      for (auto uid : selnAdd)
      {
        this->Selection.insert(uid);
      }
      for (auto uid : selnDel)
      {
        this->Selection.erase(uid);
      }
    }
    for (auto listener : this->SelectionListeners)
    {
      listener.second(this->Selection, selnAdd, selnDel, params["origin"].get<std::string>());
    }
    response["result"] = 0;
  }
  else if (method == "get selection")
  {
    response["result"] = { { "selection", this->Selection } };
  }
  else
  {
    response["error"] = { { "code", JSONRPC_METHOD_NOT_FOUND_CODE },
      { "message", JSONRPC_METHOD_NOT_FOUND_MESSAGE } };
  }

  this->SetJSONResponse(response.dump().c_str());
}

bool vtkSMTKResourceManagerWrapper::Unlisten(int handle)
{
  return this->SelectionListeners.erase(handle) > 0;
}
