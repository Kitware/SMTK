//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/appcomponents/vtkSMTKEncodeSelection.h"

#include "smtk/extension/paraview/appcomponents/pqSMTKBehavior.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKResource.h"
#include "smtk/extension/paraview/appcomponents/pqSMTKWrapper.h"

#include "smtk/extension/paraview/server/vtkSMTKResourceRepresentation.h"
#include "smtk/extension/paraview/server/vtkSMTKWrapper.h"

#include "smtk/extension/paraview/server/RespondToVTKSelection.h"
#include "smtk/extension/paraview/server/VTKSelectionResponderGroup.h"

#include "smtk/view/Selection.h"

#include "smtk/attribute/IntItem.h"

#include "smtk/resource/Resource.h"

#include "smtk/io/Logger.h"

#include "pqApplicationCore.h"
#include "pqView.h"

#include "vtkSMPropertyHelper.h"
#include "vtkSMRenderViewProxy.h"

#include "vtkInformation.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkSelection.h"
#include "vtkSelectionNode.h"

#include <algorithm>

// Change "#undef" to "#define" to enable debug printouts
#undef SMTK_DEBUG_SELECTION

vtkStandardNewMacro(vtkSMTKEncodeSelection);

vtkSMTKEncodeSelection::vtkSMTKEncodeSelection() = default;

vtkSMTKEncodeSelection::~vtkSMTKEncodeSelection() = default;

bool vtkSMTKEncodeSelection::ProcessSelection(
  vtkSelection* rawSelection,
  vtkSMRenderViewProxy* viewProxy,
  bool multipleSelectionsAllowed,
  vtkCollection* selectedRepresentations,
  vtkCollection* selectionSources,
  int modifier,
  bool selectBlocks)
{
  this->ProcessRawSelection(rawSelection, viewProxy, modifier, selectBlocks);

  bool ok = this->Superclass::ProcessSelection(
    rawSelection,
    viewProxy,
    multipleSelectionsAllowed,
    selectedRepresentations,
    selectionSources,
    modifier,
    selectBlocks);

  return ok;
}

void vtkSMTKEncodeSelection::ProcessRawSelection(
  vtkSelection* rawSelection,
  vtkSMRenderViewProxy* viewProxy,
  int modifier,
  bool selectBlocks)
{
  auto* behavior = pqSMTKBehavior::instance();
  auto* wrapper = behavior->builtinOrActiveWrapper();
  bool didModifySelection = false;

#ifdef SMTK_DEBUG_SELECTION
  std::cout << "-- paraview selection (i)***\n";
  if (!wrapper)
  {
    std::cout << "-- paraview selection (o)***\n";
    return;
  }
#else
  if (!wrapper)
  {
    return;
  }
#endif // SMTK_DEBUG_SELECTION

  // I. Empty the client-side (or active server, failing that) selection
  //    if replacing the current selection.
  std::string selnSource =
    vtkSMPropertyHelper(wrapper->getProxy(), "SelectionSource").GetAsString();
  int selnValue = vtkSMPropertyHelper(wrapper->getProxy(), "SelectedValue").GetAsInt();
  auto smtkSelection = wrapper->smtkSelection();
  if (
    !smtkSelection->currentSelection().empty() &&
    modifier == pqView::SelectionModifier::PV_SELECTION_DEFAULT)
  {
    std::set<smtk::resource::PersistentObjectPtr> blank;
    smtkSelection->modifySelection(
      blank, selnSource, selnValue, smtk::view::SelectionAction::UNFILTERED_REPLACE, false, true);
    didModifySelection = true;
  }

  // II. Aggregate selected resources by the server they reside on
  //     by traversing the selection nodes.
  std::set<smtk::resource::ResourcePtr> visited;
  int nn = rawSelection->GetNumberOfNodes();
  for (int ii = 0; ii < nn; ++ii)
  {
    vtkSelectionNode* node = rawSelection->GetNode(ii);
    vtkInformation* properties = node->GetProperties();

#ifdef SMTK_DEBUG_SELECTION
    std::cout << "  node " << (ii + 1) << "/" << nn << "\n";
    int propId = -1;
    if (properties->Has(vtkSelectionNode::PROP_ID()))
    {
      propId = properties->Get(vtkSelectionNode::PROP_ID());
      // std::cout << "    PROP_ID " << propId << "\n";
    }

    if (properties->Has(vtkSelectionNode::SOURCE_ID()))
    {
      std::cout << "    SOURCE_ID " << properties->Get(vtkSelectionNode::SOURCE_ID()) << "\n";
    }

    if (properties->Has(vtkSelectionNode::COMPOSITE_INDEX()))
    {
      std::cout << "    COMPOSITE_INDEX " << properties->Get(vtkSelectionNode::COMPOSITE_INDEX())
                << "\n";
    }
#endif

    if (properties->Has(vtkSelectionNode::SOURCE()))
    {
      auto* rr =
        vtkSMTKResourceRepresentation::SafeDownCast(properties->Get(vtkSelectionNode::SOURCE()));
      if (rr)
      {
        auto smtkResource = rr->GetResource();
        // Process each resource mentioned by any block of the selection,
        // but only process each resource once (processing will traverse
        // the entire selection, not just the current block).
        if (smtkResource && visited.find(smtkResource) == visited.end())
        {
          didModifySelection |= this->ProcessResource(
            wrapper,
            smtkResource,
            smtkSelection,
            rr,
            rawSelection,
            viewProxy,
            modifier,
            selectBlocks);
          visited.insert(smtkResource);
        }
      }
    }
  }

  // III. Notify observers if any selection transcription
  //      operation made a change to the selection.
  if (didModifySelection && smtkSelection)
  {
    // Manually notify observers that the selection has changed.
    // We do this here so that a single ParaView selection does
    // not generate many SMTK selection events.
    smtkSelection->observers()(selnSource, smtkSelection);
  }
#ifdef SMTK_DEBUG_SELECTION
  std::cout << "-- paraview selection (o)***\n";
#endif
}

bool vtkSMTKEncodeSelection::ProcessResource(
  pqSMTKWrapper* wrapper,
  const smtk::resource::ResourcePtr& resource,
  const smtk::view::SelectionPtr& smtkSelection,
  vtkSMTKResourceRepresentation* resourceRep,
  vtkSelection* rawSelection,
  vtkSMRenderViewProxy* viewProxy,
  int modifier,
  bool selectBlocks)
{
  (void)resource;
  (void)rawSelection;
  (void)smtkSelection;
  (void)modifier;
  (void)selectBlocks;

  auto* mbds = vtkMultiBlockDataSet::SafeDownCast(resourceRep->GetRenderedDataObject(0));
#ifdef SMTK_DEBUG_SELECTION
  std::cout << "Select on resource " << resource->name() << " mbds " << mbds << "\n";
#endif

  auto responderGroup = smtk::view::VTKSelectionResponderGroup(
    wrapper->smtkOperationManager(), wrapper->smtkResourceManager());
  auto operationIndices = responderGroup.operationsForResource(resource);
  return std::any_of(
    operationIndices.begin(),
    operationIndices.end(),
    [&](smtk::operation::Operation::Index operationIndex) {
      auto operation = std::dynamic_pointer_cast<smtk::view::RespondToVTKSelection>(
        wrapper->smtkOperationManager()->create(operationIndex));
      if (!operation || !operation->parameters()->associate(resource))
      {
        return false;
      }
      int mode = vtkSMPropertyHelper(viewProxy, "InteractionMode").GetAsInt();
      operation->setInteractionMode(mode);
      operation->setSMTKSelection(smtkSelection);
      operation->setVTKSelection(rawSelection);
      operation->setVTKData(mbds);
      operation->setModifier(modifier);
      operation->setSelectingBlocks(selectBlocks);
      operation->setSMTKSelectionSource("paraview");
      operation->setSMTKSelectionValue(1);
      auto result = operation->operate();
      return result->findInt("outcome")->value() ==
        static_cast<int>(smtk::operation::Operation::Outcome::SUCCEEDED);
    });
}
