//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/server/vtkSMTKWrapper.h"
#include "smtk/extension/paraview/server/RespondToVTKSelection.h"
#include "smtk/extension/paraview/server/VTKSelectionResponderGroup.h"
#include "smtk/extension/paraview/server/vtkSMTKResource.h"
#include "smtk/extension/paraview/server/vtkSMTKResourceRepresentation.h"
#include "smtk/extension/paraview/server/vtkSMTKResourceSource.h"
#include "smtk/extension/vtk/source/vtkResourceMultiBlockSource.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"

#include "smtk/common/json/jsonUUID.h"

#include "smtk/io/json/jsonComponentSet.h"
#include "smtk/io/json/jsonSelectionMap.h"

#include "smtk/model/Resource.h"

#include "smtk/operation/Manager.h"

#include "smtk/geometry/Manager.h"

#include "smtk/plugin/Manager.txx"

#include "smtk/project/Manager.h"

#include "smtk/project/old/Manager.h"

#include "smtk/view/Manager.h"
#include "smtk/view/Selection.h"

#include "smtk/resource/Component.h"
#include "smtk/resource/Manager.h"
#include "smtk/resource/Resource.h"

#include "vtkCompositeDataIterator.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkPVCompositeRepresentation.h"
#include "vtkPVSelectionSource.h"
#include "vtkSelectionNode.h"
#include "vtkUnsignedIntArray.h"

// SMTK-specific errors:

#define JSONRPC_INVALID_RESOURCE_CODE 4201
#define JSONRPC_INVALID_RESOURCE_MESSAGE "Could not obtain resource from proxy"

// Note that -32000 to -32099 are reserver for "Server error"

#define JSONRPC_INVALID_REQUEST_CODE (-32600)
#define JSONRPC_INVALID_REQUEST_MESSAGE "Invalid Request"

#define JSONRPC_METHOD_NOT_FOUND_CODE (-32601)
#define JSONRPC_METHOD_NOT_FOUND_MESSAGE "Method not found"

#define JSONRPC_INVALID_PARAMS_CODE (-32602)
#define JSONRPC_INVALID_PARAMS_MESSAGE "Invalid parameters"

#define JSONRPC_INTERNAL_ERROR_CODE (-32603)
#define JSONRPC_INTERNAL_ERROR_MESSAGE "Internal error"

#define JSONRPC_PARSE_ERROR_CODE (-32700)
#define JSONRPC_PARSE_ERROR_MESSAGE "Parse error"

using namespace nlohmann;

vtkStandardNewMacro(vtkSMTKWrapper);
vtkCxxSetObjectMacro(vtkSMTKWrapper, Representation, vtkPVDataRepresentation);

vtkSMTKWrapper::vtkSMTKWrapper()
  : ActiveResource(nullptr)
  , Managers(smtk::common::Managers::create())
  , SelectedPort(nullptr)
  , SelectionObj(nullptr)
  , JSONRequest(nullptr)
  , JSONResponse(nullptr)
  , SelectionSource("paraview")
{
  smtk::plugin::Manager::instance()->registerPluginsTo(this->Managers);

  // TODO: this logic can be removed with the introduction of the new project
  //       infrastructure
  if (
    this->Managers->contains<smtk::operation::Manager::Ptr>() &&
    this->Managers->contains<smtk::resource::Manager::Ptr>())
  {
    auto operationManager = this->Managers->get<smtk::operation::Manager::Ptr>();
    auto resourceManager = this->Managers->get<smtk::resource::Manager::Ptr>();
    this->OldProjectManager =
      smtk::project::old::Manager::create(resourceManager, operationManager);
  }

  if (this->Managers->contains<smtk::view::Selection::Ptr>())
  {
    this->SelectedValue =
      this->Managers->get<smtk::view::Selection::Ptr>()->findOrCreateLabeledValue("selected");
    this->HoveredValue =
      this->Managers->get<smtk::view::Selection::Ptr>()->findOrCreateLabeledValue("hovered");
  }
}

vtkSMTKWrapper::~vtkSMTKWrapper()
{
  this->SetJSONRequest(nullptr);
  this->SetJSONResponse(nullptr);
  this->SetActiveResource(nullptr);
  this->SetSelectionObj(nullptr);
  this->SetSelectedPort(nullptr);
}

void vtkSMTKWrapper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  indent = indent.GetNextIndent();
  os << indent << "JSONRequest: " << (this->JSONRequest ? this->JSONRequest : "null") << "\n"
     << indent << "JSONResponse: " << (this->JSONResponse ? this->JSONResponse : "null") << "\n"
     << indent << "ResourceManager: " << this->GetResourceManager() << "\n"
     << indent << "Selection: " << this->GetSelection() << "\n"
     << indent << "SelectedPort: " << this->SelectedPort << "\n"
     << indent << "SelectionObj: " << this->SelectionObj << "\n"
     << indent << "ActiveResource: " << this->ActiveResource << "\n"
     << indent << "SelectionSource: " << this->SelectionSource << "\n"
     << indent << "SelectedValue: " << this->SelectedValue << "\n"
     << indent << "HoveredValue: " << this->HoveredValue << "\n";
}

void vtkSMTKWrapper::ProcessJSON()
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
  if (method == "fetch hw selection")
  {
    this->FetchHardwareSelection(response);
  }
  else if (method == "add resource filter")
  {
    this->AddResourceFilter(response);
  }
  else if (method == "remove resource filter")
  {
    this->RemoveResourceFilter(response);
  }
  else if (method == "setup representation")
  {
    auto uid = j["params"]["resource"].get<smtk::common::UUID>();
    auto rsrc = this->GetResourceManager()->get(uid);
    auto* repr = vtkSMTKResourceRepresentation::SafeDownCast(this->Representation);
    if (repr)
    {
      repr->SetResource(rsrc);
      repr->SetWrapper(this);
      response["success"] = true;
    }
    else
    {
      vtkErrorMacro("Invalid representation!");
      response["error"] = { { "code", JSONRPC_METHOD_NOT_FOUND_CODE },
                            { "message", JSONRPC_METHOD_NOT_FOUND_MESSAGE } };
    }
  }
  else
  {
    response["error"] = { { "code", JSONRPC_INVALID_PARAMS_CODE },
                          { "message", JSONRPC_INVALID_PARAMS_MESSAGE } };
  }

  this->SetJSONResponse(response.dump().c_str());
}

void vtkSMTKWrapper::FetchHardwareSelection(json& response)
{
  // This is different than expected because there appears to be a vtkPVPostFilter
  // in between each "actual" algorithm on the client and what we get passed as
  // the port on the server side.
  std::set<smtk::resource::ComponentPtr> seln;
  auto* smtkThing = this->SelectedPort->GetProducer();
  auto* mbdsThing =
    smtkThing ? dynamic_cast<vtkMultiBlockDataSet*>(smtkThing->GetOutputDataObject(0)) : nullptr;
  auto* selnThing = this->SelectionObj->GetProducer();
  if (selnThing)
  {
    selnThing->Update();
  }
  auto* selnBlock =
    selnThing ? dynamic_cast<vtkSelection*>(selnThing->GetOutputDataObject(0)) : nullptr;
  unsigned nn = selnBlock ? selnBlock->GetNumberOfNodes() : 0;
  for (unsigned ii = 0; ii < nn; ++ii)
  {
    auto* selnNode = selnBlock->GetNode(ii);
    if (selnNode->GetContentType() == vtkSelectionNode::BLOCKS)
    {
      auto* selnList = dynamic_cast<vtkUnsignedIntArray*>(selnNode->GetSelectionList());
      unsigned mm = selnList->GetNumberOfValues();
      std::set<unsigned> blockIds;
      for (unsigned jj = 0; jj < mm; ++jj)
      {
        blockIds.insert(selnList->GetValue(jj));
      }
      if (mbdsThing)
      {
        // Go up the pipeline until we get to something that has an smtk resource:
        vtkAlgorithm* alg = smtkThing;
        while (alg && !vtkSMTKResourceSource::SafeDownCast(alg))
        { // TODO: Also stop when we get to a mesh source...
          alg = alg->GetInputAlgorithm(0, 0);
        }
        // Now we have a resource:
        smtk::resource::ResourcePtr resource = alg
          ? dynamic_cast<vtkSMTKResourceSource*>(alg)->GetVTKResource()->GetResource()
          : nullptr;
        auto* mit = mbdsThing->NewIterator();
        for (mit->InitTraversal(); !mit->IsDoneWithTraversal(); mit->GoToNextItem())
        {
          if (blockIds.find(mit->GetCurrentFlatIndex()) != blockIds.end())
          {
            auto cmp = resource->find(
              vtkResourceMultiBlockSource::GetDataObjectUUID(mit->GetCurrentMetaData()));
            if (cmp)
            {
              seln.insert(seln.end(), cmp);
            }
          }
        }
        this->GetSelection()->modifySelection(seln, "paraview", 1);
        response["selection"] = seln; // this->Selection->currentSelection();
      }
    }
  }
  std::cout << "Hardware selection!!!\n\n"
            << "  Port " << this->SelectedPort << "\n"
            << "  Seln " << this->SelectionObj << "\n"
            << "  # " << seln.size() << "  from " << this->GetSelection() << "\n\n";
}

void vtkSMTKWrapper::AddResourceFilter(json& response)
{
  // this->ActiveResource has been set. Add it to our resource manager.
  auto* rsrcThing = this->ActiveResource->GetProducer();

  auto* vtkresource = this->GetVTKResource(rsrcThing);

  if (vtkresource)
  {
    vtkresource->SetWrapper(this);
    response["result"] = {
      { "success", true },
    };
  }
  else
  {
    response["error"] = { { "code", JSONRPC_INVALID_RESOURCE_CODE },
                          { "message", JSONRPC_INVALID_RESOURCE_MESSAGE } };
  }
}

void vtkSMTKWrapper::RemoveResourceFilter(json& response)
{
  // this->ActiveResource has been set. Add it to our resource manager.
  auto* rsrcThing = this->ActiveResource->GetProducer();

  auto* vtkresource = this->GetVTKResource(rsrcThing);

  if (vtkresource)
  {
    // Remove any resource src holds from the resource manager it uses.
    vtkresource->DropResource();
    vtkresource->SetWrapper(nullptr);
    response["result"] = {
      { "success", true },
    };
  }
  else
  {
    response["error"] = { { "code", JSONRPC_INVALID_RESOURCE_CODE },
                          { "message", JSONRPC_INVALID_RESOURCE_MESSAGE } };
  }
}

vtkSMTKResource* vtkSMTKWrapper::GetVTKResource(vtkAlgorithm* algorithm)
{
  vtkSMTKResource* vtkresource = nullptr;

  if (algorithm == nullptr)
  {
    return vtkresource;
  }

  // Recursively walk up ParaView's pipeline until we encounter one of our
  // creation filters (marked by inheritence from vtkSMTKResourceSource).
  vtkAlgorithm* alg = algorithm;
  vtkSMTKResourceSource* source = nullptr;
  while (alg && !(source = vtkSMTKResourceSource::SafeDownCast(alg)))
  {
    alg = (alg->GetNumberOfInputPorts() > 0 ? alg->GetInputAlgorithm(0, 0) : nullptr);
  }
  if (source)
  {
    vtkresource = source->GetVTKResource();
  }

  // If we didn't find it, that's ok. It may be exposed within the pipeline
  // directly (as opposed to being a subfilter of vtkSMTKResourceSource).
  if (vtkresource == nullptr)
  {
    alg = algorithm;

    while (alg && !(vtkresource = vtkSMTKResource::SafeDownCast(alg)))
    {
      alg = (alg->GetNumberOfInputPorts() > 0 ? alg->GetInputAlgorithm(0, 0) : nullptr);
    }
  }

  return vtkresource;
}

vtkPVDataRepresentation* vtkSMTKWrapper::GetRepresentation()
{
  return this->Representation;
}
