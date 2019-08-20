//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/server/vtkSMTKSelectionRepresentation.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/IntItem.h"

#include "smtk/extension/paraview/server/vtkSMTKCompositeRepresentation.h"
#include "smtk/extension/paraview/server/vtkSMTKResourceRepresentation.h"
#include "smtk/extension/paraview/server/vtkSMTKWrapper.h"

#include "smtk/extension/vtk/source/vtkResourceMultiBlockSource.h"

#include "smtk/mesh/core/Resource.h"

#include "smtk/model/Resource.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/Operation.h"

#include "smtk/resource/Component.h"
#include "smtk/resource/Resource.h"

#include "smtk/view/Selection.h"

#include "vtkStreamingDemandDrivenPipeline.h"

#include "vtkAlgorithmOutput.h"
#include "vtkCellData.h"
#include "vtkCompositeDataIterator.h"
#include "vtkDataObject.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkLabeledDataMapper.h"
#include "vtkMemberFunctionCommand.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkSelection.h"
#include "vtkSelectionNode.h"
#include "vtkUnsignedIntArray.h"
#include "vtkView.h"

vtkStandardNewMacro(vtkSMTKSelectionRepresentation);
//----------------------------------------------------------------------------
vtkSMTKSelectionRepresentation::vtkSMTKSelectionRepresentation()
  : CompositeRepresentation(nullptr)
{
}

//----------------------------------------------------------------------------
vtkSMTKSelectionRepresentation::~vtkSMTKSelectionRepresentation()
{
  this->SetCompositeRepresentation(nullptr);
}

//----------------------------------------------------------------------------
void vtkSMTKSelectionRepresentation::SetCompositeRepresentation(
  vtkSMTKCompositeRepresentation* compositeRepresentation)
{
  this->CompositeRepresentation = compositeRepresentation;
}

//----------------------------------------------------------------------------
vtkSMTKResourceRepresentation* vtkSMTKSelectionRepresentation::GetResourceRepresentation() const
{
  return this->CompositeRepresentation ? vtkSMTKResourceRepresentation::SafeDownCast(
                                           this->CompositeRepresentation->GetActiveRepresentation())
                                       : nullptr;
}

//----------------------------------------------------------------------------
int vtkSMTKSelectionRepresentation::FillInputPortInformation(
  int vtkNotUsed(port), vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataObject");
  info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
  return 1;
}

//----------------------------------------------------------------------------
void vtkSMTKSelectionRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkSMTKSelectionRepresentation::SetVisibility(bool val)
{
  this->Superclass::SetVisibility(val);
}

//----------------------------------------------------------------------------
void vtkSMTKSelectionRepresentation::SetColor(double r, double g, double b)
{
  if (auto resourceRepresentation = this->GetResourceRepresentation())
  {
    resourceRepresentation->SetSelectionColor(r, g, b);
  }
}

//----------------------------------------------------------------------------
void vtkSMTKSelectionRepresentation::SetLineWidth(double val)
{
  if (auto resourceRepresentation = this->GetResourceRepresentation())
  {
    resourceRepresentation->SetSelectionLineWidth(val);
  }
}

//----------------------------------------------------------------------------
void vtkSMTKSelectionRepresentation::SetPointSize(double val)
{
  if (auto resourceRepresentation = this->GetResourceRepresentation())
  {
    resourceRepresentation->SetSelectionPointSize(val);
  }
}

//----------------------------------------------------------------------------
unsigned int vtkSMTKSelectionRepresentation::Initialize(
  unsigned int minIdAvailable, unsigned int maxIdAvailable)
{
  return this->Superclass::Initialize(minIdAvailable, maxIdAvailable);
}

//----------------------------------------------------------------------------
int vtkSMTKSelectionRepresentation::RequestData(
  vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  // Access the active selection.
  vtkSelection* activeSelection = vtkSelection::GetData(inputVector[0], 0);

  // Access the representation for the selected resource.
  vtkSMTKResourceRepresentation* resourceRepresentation = this->CompositeRepresentation
    ? vtkSMTKResourceRepresentation::SafeDownCast(
        this->CompositeRepresentation->GetActiveRepresentation())
    : nullptr;

  if (resourceRepresentation == nullptr)
  {
    // If there is no representation, there is no more processing to do.
    return this->Superclass::RequestData(request, inputVector, outputVector);
  }

  smtk::view::SelectionPtr selectionManager = resourceRepresentation->GetWrapper()->GetSelection();
  const std::string& selectionSource = resourceRepresentation->GetWrapper()->GetSelectionSource();
  int selectionValue = resourceRepresentation->GetWrapper()->GetSelectedValue();

  // Access the selected resource.
  smtk::resource::ResourcePtr resource = resourceRepresentation->GetResource();

  // Access the multiblock dataset for the selected resource.
  vtkMultiBlockDataSet* resourceDataSet =
    vtkMultiBlockDataSet::SafeDownCast(resourceRepresentation->GetRenderedDataObject(0));

  if (resource == nullptr || resourceDataSet == nullptr)
  {
    // We do not have enough information to proceed with processing the
    // selection.
    return this->Superclass::RequestData(request, inputVector, outputVector);
  }

  std::set<smtk::resource::ComponentPtr> selection;

  // Iterate over the selection nodes.
  unsigned nodeIds = activeSelection != nullptr ? activeSelection->GetNumberOfNodes() : 0;

  if (nodeIds == 0)
  {
    // Empty the selection
    selectionManager->modifySelection(selection, selectionSource, selectionValue);
  }

  for (unsigned nodeId = 0; nodeId < nodeIds; ++nodeId)
  {
    vtkSelectionNode* selectionNode = activeSelection->GetNode(nodeId);
    // If the selection mode is for blocks, correlate blocks to component IDs.
    if (selectionNode->GetContentType() == vtkSelectionNode::BLOCKS)
    {
      // Access the list of selected blocks.
      auto selectionList = dynamic_cast<vtkUnsignedIntArray*>(selectionNode->GetSelectionList());
      unsigned selectionIds = selectionList->GetNumberOfValues();

      // Convert the list of selected blocks into a set to avoid redundancy.
      std::set<unsigned> blockIds;
      for (unsigned selectionId = 0; selectionId < selectionIds; ++selectionId)
      {
        blockIds.insert(selectionList->GetValue(selectionId));
      }

      // For each block in the resource dataset, check if the block is in our
      // set of selected blocks.
      auto blockIterator = resourceDataSet->NewIterator();
      for (blockIterator->InitTraversal(); !blockIterator->IsDoneWithTraversal();
           blockIterator->GoToNextItem())
      {
        // If the block is selected...
        if (blockIds.find(blockIterator->GetCurrentFlatIndex()) != blockIds.end())
        {
          // ...find the the corresponding component by UUID.
          auto component = resource->find(
            vtkResourceMultiBlockSource::GetDataObjectUUID(blockIterator->GetCurrentMetaData()));

          // If the component is found...
          if (component)
          {
            // ...insert it into the selection.
            selection.insert(selection.end(), component);
          }
        }
      }

      // Update the selection to include the components that were selected via
      // the render window.
      selectionManager->modifySelection(selection, selectionSource, selectionValue);

      // Clean up the block iterator, since we created it for the above
      // traversal.
      blockIterator->Delete();
    }
  }

  return this->Superclass::RequestData(request, inputVector, outputVector);
}
