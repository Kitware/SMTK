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

#include "smtk/extension/paraview/server/vtkSMTKModelReader.h"
#include "smtk/extension/vtk/source/vtkModelMultiBlockSource.h"

#include "smtk/io/jsonComponentSet.h"
#include "smtk/io/jsonSelectionMap.h"
#include "smtk/io/jsonUUID.h"

#include "smtk/model/Manager.h"

#include "smtk/operation/Manager.h"

#include "smtk/resource/Component.h"
#include "smtk/resource/Manager.h"
#include "smtk/resource/Resource.h"
#include "smtk/resource/SelectionManager.h"

#include "vtkCompositeDataIterator.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkPVSelectionSource.h"
#include "vtkSelectionNode.h"
#include "vtkUnsignedIntArray.h"

// SMTK-specific errors:

#define JSONRPC_INVALID_RESOURCE_CODE 4201
#define JSONRPC_INVALID_RESOURCE_MESSAGE "Could not obtain resource from proxy"

// Note that -32000 to -32099 are reserver for "Server error"

#define JSONRPC_INVALID_REQUEST_CODE -32600
#define JSONRPC_INVALID_REQUEST_MESSAGE "Invalid Request"

#define JSONRPC_METHOD_NOT_FOUND_CODE -32601
#define JSONRPC_METHOD_NOT_FOUND_MESSAGE "Method not found"

#define JSONRPC_INVALID_PARAMS_CODE -32602
#define JSONRPC_INVALID_PARAMS_MESSAGE "Invalid parameters"

#define JSONRPC_INTERNAL_ERROR_CODE -32603
#define JSONRPC_INTERNAL_ERROR_MESSAGE "Internal error"

#define JSONRPC_PARSE_ERROR_CODE -32700
#define JSONRPC_PARSE_ERROR_MESSAGE "Parse error"

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
    s_instance->Register(nullptr);
    atexit(destroyInstance);
  }
  return s_instance;
}

vtkSMTKResourceManagerWrapper* vtkSMTKResourceManagerWrapper::Instance()
{
  return s_instance;
}

vtkSMTKResourceManagerWrapper::vtkSMTKResourceManagerWrapper()
  : ActiveResource(nullptr)
  , SelectedPort(nullptr)
  , SelectionObj(nullptr)
  , JSONRequest(nullptr)
  , JSONResponse(nullptr)
  , SelectionSource("paraview")
{
  this->OperationManager = smtk::operation::Manager::create();
  this->ResourceManager = smtk::resource::Manager::create();
  this->SelectionManager = smtk::resource::SelectionManager::create();
  this->SelectionManager->setDefaultAction(smtk::resource::SelectionAction::FILTERED_REPLACE);
  this->SelectedValue = this->SelectionManager->findOrCreateLabeledValue("selected");
  this->HoveredValue = this->SelectionManager->findOrCreateLabeledValue("hovered");
  this->SelectionListener = this->SelectionManager->listenToSelectionEvents(
    [](const std::string& src, smtk::resource::SelectionManager::Ptr selnMgr) {
      std::cout << "--- RsrcManagerWrapper " << selnMgr << " src \"" << src << "\":\n";
      selnMgr->visitSelection([](smtk::resource::ComponentPtr comp, int value) {
        auto modelComp = smtk::dynamic_pointer_cast<smtk::model::Entity>(comp);
        if (modelComp)
        {
          smtk::model::EntityRef ent(modelComp->modelResource(), modelComp->id());
          std::cout << "  " << comp->id() << ": " << value << ",  " << ent.flagSummary() << ": "
                    << ent.name() << "\n";
        }
        else
        {
          std::cout << "  " << comp->id() << ":  " << value << "\n";
        }
      });
      std::cout << "----\n";
      std::cout.flush();
    },
    true);
}

vtkSMTKResourceManagerWrapper::~vtkSMTKResourceManagerWrapper()
{
  this->SelectionManager->unlisten(this->SelectionListener);
  this->SetJSONRequest(nullptr);
  this->SetJSONResponse(nullptr);
  this->SetActiveResource(nullptr);
  this->SetSelectionObj(nullptr);
  this->SetSelectedPort(nullptr);
}

void vtkSMTKResourceManagerWrapper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  indent = indent.GetNextIndent();
  os << indent << "JSONRequest: " << (this->JSONRequest ? this->JSONRequest : "null") << "\n"
     << indent << "JSONResponse: " << (this->JSONResponse ? this->JSONResponse : "null") << "\n"
     << indent << "ResourceManager: " << this->ResourceManager.get() << "\n"
     << indent << "SelectionManager: " << this->SelectionManager.get() << "\n"
     << indent << "SelectedPort: " << this->SelectedPort << "\n"
     << indent << "SelectionObj: " << this->SelectionObj << "\n"
     << indent << "ActiveResource: " << this->ActiveResource << "\n"
     << indent << "SelectionSource: " << this->SelectionSource << "\n"
     << indent << "SelectedValue: " << this->SelectedValue << "\n"
     << indent << "HoveredValue: " << this->HoveredValue << "\n";
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
  if (method == "fetch hw selection")
  {
    this->FetchHardwareSelection(response);
  }
  else if (method == "add resource")
  {
    this->AddResource(response);
  }
  else if (method == "remove resource")
  {
    this->RemoveResource(response);
  }
  else
  {
    response["error"] = { { "code", JSONRPC_METHOD_NOT_FOUND_CODE },
      { "message", JSONRPC_METHOD_NOT_FOUND_MESSAGE } };
  }

  this->SetJSONResponse(response.dump().c_str());
}

void vtkSMTKResourceManagerWrapper::FetchHardwareSelection(json& response)
{
  // This is different than expected because there appears to be a vtkPVPostFilter
  // in between each "actual" algorithm on the client and what we get passed as
  // the port on the server side.
  std::set<smtk::resource::ComponentPtr> seln;
  //auto smtkThing = dynamic_cast<vtkSMTKModelReader*>(this->SelectedPort->GetProducer());
  auto smtkThing = this->SelectedPort->GetProducer();
  auto mbdsThing =
    smtkThing ? dynamic_cast<vtkMultiBlockDataSet*>(smtkThing->GetOutputDataObject(0)) : nullptr;
  auto selnThing = this->SelectionObj->GetProducer();
  if (selnThing)
  {
    selnThing->Update();
  }
  auto selnBlock =
    selnThing ? dynamic_cast<vtkSelection*>(selnThing->GetOutputDataObject(0)) : nullptr;
  unsigned nn = selnBlock ? selnBlock->GetNumberOfNodes() : 0;
  for (unsigned ii = 0; ii < nn; ++ii)
  {
    auto selnNode = selnBlock->GetNode(ii);
    if (selnNode->GetContentType() == vtkSelectionNode::BLOCKS)
    {
      auto selnList = dynamic_cast<vtkUnsignedIntArray*>(selnNode->GetSelectionList());
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
        while (alg && !vtkSMTKModelReader::SafeDownCast(alg))
        { // TODO: Also stop when we get to a mesh source...
          alg = alg->GetInputAlgorithm(0, 0);
        }
        // Now we have a resource:
        smtk::model::ManagerPtr mgr = alg
          ? dynamic_cast<vtkSMTKModelReader*>(alg)->GetModelSource()->GetModelManager()
          : nullptr;
        auto mit = mbdsThing->NewIterator();
        for (mit->InitTraversal(); !mit->IsDoneWithTraversal(); mit->GoToNextItem())
        {
          if (blockIds.find(mit->GetCurrentFlatIndex()) != blockIds.end())
          {
            auto ent = vtkModelMultiBlockSource::GetDataObjectEntityAs<smtk::model::EntityRef>(
              mgr, mit->GetCurrentMetaData());
            auto cmp = ent.component();
            if (cmp)
            {
              seln.insert(seln.end(), cmp);
            }
          }
        }
        this->SelectionManager->modifySelection(seln, "paraview", 1);
        response["selection"] = seln; // this->SelectionManager->currentSelection();
      }
    }
  }
  std::cout << "Hardware selection!!!\n\n"
            << "  Port " << this->SelectedPort << "\n"
            << "  Seln " << this->SelectionObj << "\n"
            << "  # " << seln.size() << "  from " << this->SelectionManager << "\n\n";
}

void vtkSMTKResourceManagerWrapper::AddResource(json& response)
{
  // this->ActiveResource has been set. Add it to our resource manager.
  bool found = false;
  auto rsrcThing = this->ActiveResource->GetProducer();
  if (rsrcThing)
  {
    vtkAlgorithm* alg = rsrcThing;
    while (alg && !vtkSMTKModelReader::SafeDownCast(alg))
    { // TODO: Also stop when we get to a mesh/attrib/etc source...
      alg = alg->GetInputAlgorithm(0, 0);
    }
    auto src = vtkSMTKModelReader::SafeDownCast(alg);
    if (src)
    {
      src->ObserveResourceChanges([this](smtk::model::ManagerPtr rsrc, bool adding) {
        if (rsrc)
        {
          if (adding)
          {
            std::cout << "  Adding resource   " << rsrc->id() << " loc " << rsrc->location()
                      << "\n";
            this->ResourceManager->add(rsrc);
          }
          else
          {
            std::cout << "  Removing resource " << rsrc->id() << " loc " << rsrc->location()
                      << "\n";
            this->ResourceManager->remove(rsrc);
          }
        }
      });
      found = true;
      response["result"] = {
        { "success", true },
      };
    }
  }
  if (!found)
  {
    response["error"] = { { "code", JSONRPC_INVALID_RESOURCE_CODE },
      { "message", JSONRPC_INVALID_RESOURCE_MESSAGE } };
  }
}

void vtkSMTKResourceManagerWrapper::RemoveResource(json& response)
{
  // this->ActiveResource has been set. Add it to our resource manager.
  bool found = false;
  auto rsrcThing = this->ActiveResource->GetProducer();
  if (rsrcThing)
  {
    vtkAlgorithm* alg = rsrcThing;
    while (alg && !vtkSMTKModelReader::SafeDownCast(alg))
    { // TODO: Also stop when we get to a mesh/attrib/etc source...
      alg = alg->GetInputAlgorithm(0, 0);
    }
    auto src = vtkSMTKModelReader::SafeDownCast(alg);
    if (src)
    {
      src->UnobserveResourceChanges();
      auto rsrc = src->GetSMTKResource();
      this->ResourceManager->remove(rsrc);
      response["result"] = {
        { "success", true },
      };
      found = true;
    }
  }
  if (!found)
  {
    response["error"] = { { "code", JSONRPC_INVALID_RESOURCE_CODE },
      { "message", JSONRPC_INVALID_RESOURCE_MESSAGE } };
  }
}
