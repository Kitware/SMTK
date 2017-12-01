//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/pqSMTKResourceManager.h"

// SMTK
#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKResource.h"
#include "smtk/extension/paraview/server/vtkSMSMTKResourceManagerProxy.h"
#include "smtk/extension/paraview/server/vtkSMTKModelReader.h" // TODO: remove need for me
#include "smtk/extension/paraview/server/vtkSMTKResourceManagerWrapper.h"

#include "smtk/extension/vtk/source/vtkModelMultiBlockSource.h" // TODO: remove need for me

#include "smtk/view/Selection.h"

#include "smtk/io/Logger.h"

#include "smtk/resource/Component.h"
#include "smtk/resource/Manager.h"
#include "smtk/resource/Resource.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/Operator.h"

#include "smtk/model/Entity.h"
#include "smtk/model/EntityRef.h"

// PV Client side
#include "pqApplicationCore.h"
#include "pqOutputPort.h"
#include "pqSelectionManager.h"

#include "vtkSMSourceProxy.h"

// Server side (TODO: Remove need for me)
#include "vtkCompositeDataIterator.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkPVSelectionSource.h"
#include "vtkSelection.h"
#include "vtkSelectionNode.h"
#include "vtkUnsignedIntArray.h"

#include <iostream>

pqSMTKResourceManager::pqSMTKResourceManager(const QString& regGroup, const QString& regName,
  vtkSMProxy* proxy, pqServer* server, QObject* parent)
  : Superclass(regGroup, regName, proxy, server, parent)
{
  // I. Listen for PV selections and convert them to SMTK selections
  std::cout << "pqResourceManager ctor " << parent << "\n";
  bool listening = false;
  auto app = pqApplicationCore::instance();
  if (app)
  {
    auto pvSelnMgr = qobject_cast<pqSelectionManager*>(app->manager("SelectionManager"));
    if (pvSelnMgr)
    {
      if (QObject::connect(pvSelnMgr, SIGNAL(selectionChanged(pqOutputPort*)), this,
            SLOT(paraviewSelectionChanged(pqOutputPort*))))
      {
        listening = true;
      }
    }
  }
  if (!listening)
  {
    smtkErrorMacro(smtk::io::Logger::instance(),
      "Could not connect SMTK resource manager to ParaView selection manager.");
  }

  // II. Listen for operation events and signal them.
  //     Note that what we **should** be doing is listening for these
  //     events on a client-side operation manager used to forward
  //     operations to the server. What we in fact do only works for
  //     the built-in mode. TODO: Fix this.
  auto pxy = vtkSMSMTKResourceManagerProxy::SafeDownCast(this->getProxy());
  auto wrapper = vtkSMTKResourceManagerWrapper::SafeDownCast(pxy->GetClientSideObject());
  if (wrapper)
  {
    /*
    wrapper->GetManager()->observe([this](
        smtk::resource::Event event,
        const smtk::resource::ResourcePtr& rsrc)
      {
        switch (event)
        {
        case smtk::resource::Event::RESOURCE_ADDED: emit resourceAdded(rsrc); break;
        case smtk::resource::Event::RESOURCE_REMOVED: emit resourceRemoved(rsrc); break;
        }
      }
    );
    */
    wrapper->GetOperationManager()->observe(
      [this](smtk::operation::Operator::Ptr oper, smtk::operation::Operator::EventType event,
        smtk::operation::Operator::Result result) -> int {
        emit operationEvent(oper, event, result);
        return 0;
      });
  }
  pqSMTKBehavior::instance()->addPQProxy(this);
}

pqSMTKResourceManager::~pqSMTKResourceManager()
{
  std::cout << "pqResourceManager dtor\n";
}

vtkSMSMTKResourceManagerProxy* pqSMTKResourceManager::smtkProxy() const
{
  return vtkSMSMTKResourceManagerProxy::SafeDownCast(this->getProxy());
}

smtk::resource::ManagerPtr pqSMTKResourceManager::smtkResourceManager() const
{
  return this->smtkProxy()->GetManager();
}

smtk::operation::ManagerPtr pqSMTKResourceManager::smtkOperationManager() const
{
  return this->smtkProxy()->GetOperationManager();
}

smtk::view::SelectionPtr pqSMTKResourceManager::smtkSelection() const
{
  return this->smtkProxy()->GetSelection();
}

void pqSMTKResourceManager::addResource(pqSMTKResource* rsrc)
{
  auto pxy = this->smtkProxy();
  if (pxy)
  {
    pxy->AddResourceProxy(rsrc->getSourceProxy());
    emit resourceAdded(rsrc);
  }
}

void pqSMTKResourceManager::removeResource(pqSMTKResource* rsrc)
{
  auto pxy = this->smtkProxy();
  if (pxy)
  {
    emit resourceRemoved(rsrc);
    pxy->RemoveResourceProxy(rsrc->getSourceProxy());
  }
}

void pqSMTKResourceManager::paraviewSelectionChanged(pqOutputPort* port)
{
  std::cout << "PV selection change, port " << port << "\n";
  /* TODO: This needs to be completed to work with separate server processes.
  auto pxy = this->wrapperProxy();
  (void)pxy;
  if (!port)
  {
    pxy->modifySelection(seln, m_selectionSource, m_selectedValue);
    std::cout << "FIXME: Reset selection when port is null\n";
    return;
  }
  // I. Get vtkSelection source proxy ID
  auto dataInput = port->getSourceProxy();
  auto selnInput = port->getSelectionInput();
  // II. Set it as an input on the vtkSMTKResourceManagerWrapper's SelectionPort
  pxy->SetSelectedPortProxy(dataInput);
  pxy->SetSelectionObjProxy(selnInput);
  // III. Send a JSON request to the vtkSMTKResourceManagerWrapper telling it to grab the selection.
  pxy->FetchHardwareSelection();
  // IV. Update the client-side selection.
  // TODO
  */
  // TODO: This only works in built-in mode.
  auto wrapper =
    vtkSMTKResourceManagerWrapper::SafeDownCast(this->getProxy()->GetClientSideObject());
  auto selnMgr = wrapper ? wrapper->GetSelection() : nullptr;
  if (!selnMgr)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "No selection manager!");
    return;
  }
  std::string selnSrc = wrapper->GetSelectionSource();
  int selnVal = wrapper->GetSelectedValue();
  std::set<smtk::resource::ComponentPtr> seln;
  if (!port)
  {
    //std::cout << "Selection empty\n";
    selnMgr->modifySelection(seln, selnSrc, selnVal);
  }
  else
  {
    auto dataInput = port->getSourceProxy();
    auto dataThing = dataInput->GetClientSideObject();
    auto smtkThing = dynamic_cast<vtkSMTKModelReader*>(dataThing);
    auto mbdsThing = smtkThing ? smtkThing->GetOutput() : nullptr;
    auto selnInput = port->getSelectionInput();
    auto selnThing = dynamic_cast<vtkPVSelectionSource*>(selnInput->GetClientSideObject());
    if (selnThing)
    {
      selnThing->Update();
    }
    auto selnBlock = selnThing ? selnThing->GetOutput() : nullptr;
    unsigned nn = selnBlock ? selnBlock->GetNumberOfNodes() : 0;
    /*
    std::cout
      << "Selection input " << selnInput
      << " client side " << selnThing
      << " seln nodes " << nn
      << " data thing " << dataThing << " " << dataThing->GetClassName()
      << "\n";
     */
    for (unsigned ii = 0; ii < nn; ++ii)
    {
      auto selnNode = selnBlock->GetNode(ii);
      if (selnNode->GetContentType() == vtkSelectionNode::BLOCKS)
      {
        //vtkIndent indent;
        //selnNode->PrintSelf(std::cout, indent);
        /*
        auto selnInfo = selnNode->GetProperties();
        auto selnProp = selnInfo->Has(vtkSelectionNode::vtkSelectionNode::PROP_ID()) ?
          selnInfo->Get(vtkSelectionNode::PROP_ID()) : -2;
        auto selnHLvl = selnInfo->Has(vtkSelectionNode::HIERARCHICAL_LEVEL()) ?
           selnInfo->Get(vtkSelectionNode::HIERARCHICAL_LEVEL()) : -2;
        auto selnHier = selnInfo->Has(vtkSelectionNode::HIERARCHICAL_INDEX()) ?
           selnInfo->Get(vtkSelectionNode::HIERARCHICAL_INDEX()) : -2;
        auto selnComp = selnInfo->Has(vtkSelectionNode::COMPOSITE_INDEX()) ?
          selnInfo->Get(vtkSelectionNode::COMPOSITE_INDEX()) : -2;
        auto selnSrcI = selnInfo->Has(vtkSelectionNode::SOURCE_ID()) ?
          selnInfo->Get(vtkSelectionNode::SOURCE_ID()) : -2;
        auto selnDObj = selnInfo->Get(vtkSelectionNode::SOURCE());
        std::cout << "  seln prop id " << selnProp << " hier " << selnHLvl << " " << selnHier << " composite id " << selnComp << " src id " << selnSrcI << " data obj " << (selnDObj ? selnDObj->GetClassName() : "null") << "\n";
        */
        auto selnList = dynamic_cast<vtkUnsignedIntArray*>(selnNode->GetSelectionList());
        unsigned mm = selnList->GetNumberOfValues();
        //std::cout << "  seln list " << mm << " (";
        std::set<unsigned> blockIds;
        for (unsigned jj = 0; jj < mm; ++jj)
        {
          //std::cout << " " << selnList->GetValue(jj);
          blockIds.insert(selnList->GetValue(jj));
        }
        //std::cout << " )\n";
        if (mbdsThing)
        {
          smtk::model::ManagerPtr mgr = smtkThing->GetModelSource()->GetModelManager();
          //std::cout << "  selected model entities:";
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
              //std::cout << ", " << ent.name();
            }
          }
          //std::cout << "\n";
          selnMgr->modifySelection(seln, selnSrc, selnVal);
        }
      }
    }
  }
}
