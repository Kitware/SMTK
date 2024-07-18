//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/mesh/VTKMeshCellSelection.h"
#include "smtk/extension/paraview/mesh/VTKMeshCellSelection_xml.h"

#include "smtk/extension/vtk/io/mesh/MeshIOVTK.h"

#include "smtk/extension/vtk/source/vtkResourceMultiBlockSource.h"

#include "smtk/view/Selection.h"

#include "smtk/mesh/core/Resource.h"
#include "smtk/mesh/operators/DeleteMesh.h"
#include "smtk/mesh/operators/SelectCells.h"
#include "smtk/mesh/resource/Selection.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ComponentItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/operation/Manager.h"
#include "smtk/operation/MarkGeometry.h"

#include "smtk/io/Logger.h"

#include "vtkCellData.h"
#include "vtkCompositeDataIterator.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkPolyData.h"
#include "vtkSelection.h"
#include "vtkSelectionNode.h"
#include "vtkUnsignedIntArray.h"

namespace smtk
{
namespace view
{

VTKMeshCellSelection::VTKMeshCellSelection() = default;

VTKMeshCellSelection::~VTKMeshCellSelection() = default;

bool VTKMeshCellSelection::transcribeCellIdSelection(Result& result)
{
  bool didModify = false;
  if (this->selectingBlocks())
  {
    return didModify;
  }

  auto assoc = this->parameters()->associations();
  smtk::resource::ResourcePtr resource = assoc->valueAs<smtk::resource::Resource>();
  // Currently, cell selection is only enabled for meshes and models that
  // have mesh tessellations.
  smtk::mesh::Resource::Ptr meshResource =
    std::dynamic_pointer_cast<smtk::mesh::Resource>(resource);
  if (meshResource == nullptr)
  {
    smtk::model::Resource::Ptr modelResource =
      std::dynamic_pointer_cast<smtk::model::Resource>(resource);
    if (modelResource)
    {
      meshResource = modelResource->meshTessellations();
    }
  }
  if (!resource)
  {
    smtkErrorMacro(smtk::io::Logger::instance(), "No associated resource for mesh selection.");
    return false;
  }

  auto selnMgr = this->smtkSelection();
  auto* selnBlock = this->vtkSelection();
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
    auto* selnNode = selnBlock->GetNode(ii);
    if (selnNode->GetContentType() == vtkSelectionNode::INDICES)
    {
      vtkInformation* selProperties = selnNode->GetProperties();

      // Ids for the multiblocks selected
      //
      // TODO: handle meshsets from multiple processes (accessed using
      //       selProperties->Has(vtkSelectionNode::PROCESS_ID())
      std::map<unsigned, std::vector<unsigned>> selectedVTKCellsByMeshset;

      // Extract selected vtk composite index and cell ids
      vtkIdTypeArray* idList = vtkIdTypeArray::SafeDownCast(selnNode->GetSelectionList());
      if (idList)
      {
        vtkIdType numIDs = idList->GetNumberOfTuples();
        vtkIdType composite_index = 0;
        // The composite index corresponds to the block that contains the
        // selected cell.
        if (selProperties->Has(vtkSelectionNode::COMPOSITE_INDEX()))
        {
          composite_index = selProperties->Get(vtkSelectionNode::COMPOSITE_INDEX());
        }

        // The idList values correspond to the selected cells within the
        // block.
        for (vtkIdType cc = 0; cc < numIDs; cc++)
        {
          selectedVTKCellsByMeshset[composite_index].push_back(idList->GetValue(cc));
        }
      }

      smtk::mesh::SelectCells::Ptr selectCells;
      if (auto operationManager = this->manager())
      {
        selectCells = operationManager->create<smtk::mesh::SelectCells>();
      }
      else
      {
        selectCells = smtk::mesh::SelectCells::create();
      }

      selectCells->parameters()->associate(meshResource);

      smtk::attribute::StringItem::Ptr cells = selectCells->parameters()->findString("cell ids");

      // TODO: is there no way to access the appropriate dataset by index
      // (as opposed to iterating the entire composite dataset)?

      std::set<smtk::resource::ComponentPtr> selection;
      // For each block in the selected vtkMultiBlock...
      vtkCompositeDataIterator* blockIterator = this->vtkData()->NewIterator();
      for (blockIterator->InitTraversal(); !blockIterator->IsDoneWithTraversal();
           blockIterator->GoToNextItem())
      {
        // ...if the block index corresponds to the selected composite
        // index...
        auto selectedVTKCellsIt =
          selectedVTKCellsByMeshset.find(blockIterator->GetCurrentFlatIndex());
        if (selectedVTKCellsIt != selectedVTKCellsByMeshset.end())
        {
          // ...access the cell data corresponding to the mapping between
          // VTK cells and SMTK cells.
          std::vector<unsigned>& selectedVTKCells = selectedVTKCellsIt->second;

          // First we cast the dataset into something we can use.
          vtkPolyData* vtkMeshSet =
            vtkPolyData::SafeDownCast(blockIterator->GetCurrentDataObject());
          if (vtkMeshSet == nullptr)
          {
            continue;
          }

          // Then we access the mapping between VTK cells and SMTK cells.
          vtkIdTypeArray* cellHandles =
            vtkIdTypeArray::SafeDownCast(vtkMeshSet->GetCellData()->GetScalars(
              smtk::extension::vtk::io::mesh::MeshIOVTK::CellHandlesName));

          if (cellHandles == nullptr)
          {
            continue;
          }

          // Copy the mapped SMTK cells into a HandleRange.
          for (unsigned& vtkCell : selectedVTKCells)
          {
            cells->appendValue(std::to_string(cellHandles->GetValue(vtkCell)));
          }
        }
      }

      smtk::operation::Operation::Result selectionResult = selectCells->operate(this->childKey());
      smtk::attribute::ComponentItem::Ptr created = selectionResult->findComponent("created");
      selection.insert(created->value());
      result->findComponent("created")->appendValue(created->value());
      smtk::operation::MarkGeometry().markModified(created->value());

      created->reset();

      didModify |= selnMgr->modifySelection(
        selection,
        m_smtkSelectionSource,
        m_smtkSelectionValue,
        SelectionAction::FILTERED_ADD,
        /* bitwise */ true,
        /* notify */ false);

      // Clean up the block iterator, since we created it for the above
      // traversal.
      blockIterator->Delete();
    }
    else
    {
      //std::cout << "Failing to convert due to unexpected selection node.\n";
      didModify = false;
      break;
    }
  }

  return didModify;
}

VTKMeshCellSelection::Result VTKMeshCellSelection::operateInternal()
{
  auto result = this->createResult(smtk::operation::Operation::Outcome::SUCCEEDED);
  bool worked = this->transcribeCellIdSelection(result);
  if (!worked)
  {
    result->findInt("outcome")->setValue(
      static_cast<int>(smtk::operation::Operation::Outcome::FAILED));
  }
  return result;
}

const char* VTKMeshCellSelection::xmlDescription() const
{
  return VTKMeshCellSelection_xml;
}

} //namespace view
} // namespace smtk
