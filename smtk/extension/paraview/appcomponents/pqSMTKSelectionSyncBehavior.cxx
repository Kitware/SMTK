//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/pqSMTKSelectionSyncBehavior.h"

// SMTK
#include "smtk/extension/paraview/server/vtkSMTKModelReader.h"
#include "smtk/extension/vtk/source/vtkModelMultiBlockSource.h"
#include "smtk/model/EntityRef.h"
#include "smtk/resource/SelectionManager.h"

// Client side
#include "pqApplicationCore.h"
#include "pqOutputPort.h"
#include "pqPVApplicationCore.h"
#include "pqSelectionManager.h"
#include "pqServerManagerModel.h"

// Server side
#include "vtkPVSelectionSource.h"
#include "vtkSMSourceProxy.h"

// VTK
#include "vtkAbstractArray.h"
#include "vtkCompositeDataIterator.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkSelection.h"
#include "vtkSelectionNode.h"
#include "vtkUnsignedIntArray.h"

using namespace smtk;

pqSMTKSelectionSyncBehavior::pqSMTKSelectionSyncBehavior(
  QObject* parent, smtk::resource::SelectionManagerPtr mgr)
  : Superclass(parent)
  , m_connected(false)
  , m_selectionManager(mgr)
{
  // Configure the selection manager:
  m_selectionManager->setDefaultAction(smtk::resource::SelectionAction::FILTERED_REPLACE);
  m_selectedValue = m_selectionManager->findOrCreateLabeledValue("selected");
  m_hoveredValue = m_selectionManager->findOrCreateLabeledValue("hovered");
  m_selectionSource = "paraview";
  while (!m_selectionManager->registerSelectionSource(m_selectionSource))
  {
    m_selectionSource += "X";
  }
  m_selectionListener = m_selectionManager->listenToSelectionEvents(
    [](const std::string& src, smtk::resource::SelectionManager::Ptr selnMgr) {
      std::cout << "Selection modified by: \"" << src << "\"\n";
      selnMgr->visitSelection([](smtk::resource::ComponentPtr comp, int value) {
        auto modelComp = dynamic_pointer_cast<smtk::model::Entity>(comp);
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
      std::cout.flush();
    },
    true);

  // Blech: pqApplicationCore doesn't have the selection manager yet,
  // so wait until we hear that the server is ready to make the connection.
  // We can't have a selection before the first connection, anyway.
  auto pqCore = pqApplicationCore::instance();
  if (pqCore)
  {
    QObject::connect(pqCore->getServerManagerModel(), SIGNAL(serverReady(pqServer*)), this,
      SLOT(connectSelectionManagers()));
  }
}

pqSMTKSelectionSyncBehavior::~pqSMTKSelectionSyncBehavior()
{
}

void pqSMTKSelectionSyncBehavior::connectSelectionManagers()
{
  if (m_connected)
  {
    return;
  }

  auto pqCore = pqApplicationCore::instance();
  if (pqCore)
  {
    auto smgr =
      qobject_cast<pqSelectionManager*>(pqApplicationCore::instance()->manager("SelectionManager"));
    if (smgr)
    {
      m_connected = true;
      QObject::connect(smgr, SIGNAL(selectionChanged(pqOutputPort*)), this,
        SLOT(paraviewSelectionChanged(pqOutputPort*)));
    }
  }
}

void pqSMTKSelectionSyncBehavior::paraviewSelectionChanged(pqOutputPort* port)
{
  std::set<smtk::resource::ComponentPtr> seln;
  if (!port)
  {
    //std::cout << "Selection empty\n";
    m_selectionManager->modifySelection(seln, m_selectionSource, m_selectedValue);
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
          model::ManagerPtr mgr = smtkThing->GetModelSource()->GetModelManager();
          //std::cout << "  selected model entities:";
          auto mit = mbdsThing->NewIterator();
          for (mit->InitTraversal(); !mit->IsDoneWithTraversal(); mit->GoToNextItem())
          {
            if (blockIds.find(mit->GetCurrentFlatIndex()) != blockIds.end())
            {
              auto ent = vtkModelMultiBlockSource::GetDataObjectEntityAs<model::EntityRef>(
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
          m_selectionManager->modifySelection(seln, m_selectionSource, m_selectedValue);
        }
      }
    }
  }
}
