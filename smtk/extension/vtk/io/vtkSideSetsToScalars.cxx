//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/extension/vtk/io/vtkSideSetsToScalars.h"
#include "vtkCellData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkPointData.h"
#include "vtkUnsignedCharArray.h"
#include "vtkUnstructuredGrid.h"

vtkStandardNewMacro(vtkSideSetsToScalars);

void vtkSideSetsToScalars::SetScalarArrayName(const std::string& name)
{
  if (this->ScalarArrayName == name)
  {
    return;
  }
  this->ScalarArrayName = name;
  this->Modified();
}

const std::string& vtkSideSetsToScalars::GetScalarArrayName() const
{
  return this->ScalarArrayName;
}

int vtkSideSetsToScalars::RequestData(
  vtkInformation* /*request*/,
  vtkInformationVector** inputVec,
  vtkInformationVector* outputVec)
{
  // Get the inputs
  vtkInformation* inInfo1 = inputVec[0]->GetInformationObject(0);
  vtkMultiBlockDataSet* inputMasterBlock =
    vtkMultiBlockDataSet::SafeDownCast(inInfo1->Get(vtkDataObject::DATA_OBJECT()));
  vtkInformation* inInfo2 = inputVec[1]->GetInformationObject(0);
  vtkMultiBlockDataSet* inputGroupBlock =
    vtkMultiBlockDataSet::SafeDownCast(inInfo2->Get(vtkDataObject::DATA_OBJECT()));

  // Get the outputs
  vtkInformation* outInfo = outputVec->GetInformationObject(0);
  vtkMultiBlockDataSet* outputMasterBlock =
    vtkMultiBlockDataSet::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

  outputMasterBlock->ShallowCopy(inputMasterBlock);

  // Foreword: Med files separate points and cells. Each has their own separate global ids for good reason
  // this means their global ids can overlap (ie id=0 could refer to cell 0 or point 0), so they must be handled
  // separately. Additionally points global ids are actually put in as cell global ids where the cell is VTK_VERTEX

  // Extract datasets and verify the contents
  std::vector<vtkSmartPointer<vtkUnstructuredGrid>> masterInputs;
  for (unsigned int i = 0; i < inputMasterBlock->GetNumberOfBlocks(); i++)
  {
    vtkSmartPointer<vtkUnstructuredGrid> input =
      vtkUnstructuredGrid::SafeDownCast(inputMasterBlock->GetBlock(i));
    if (input == nullptr)
    {
      vtkWarningMacro(
        << "input[0] multi block of vtkSideSetsToScalars contains block that is not "
           "an unstructured grid.");
      return 1;
    }
    masterInputs.push_back(input);
  }
  std::vector<vtkSmartPointer<vtkUnstructuredGrid>> groupInputs;
  std::vector<std::string> groupNames;
  for (unsigned int i = 0; i < inputGroupBlock->GetNumberOfBlocks(); i++)
  {
    vtkSmartPointer<vtkUnstructuredGrid> input =
      vtkUnstructuredGrid::SafeDownCast(inputGroupBlock->GetBlock(i));
    if (input == nullptr)
    {
      vtkWarningMacro(
        << "input[1] multi block of vtkSideSetsToScalars contains block that is not "
           "an unstructured grid.");
      return 1;
    }

    const char* name = inputGroupBlock->GetMetaData(i)->Get(vtkCompositeDataSet::NAME());
    groupNames.emplace_back(name ? name : "side-set");
    groupInputs.push_back(input);
  }

  // Map of cell ids to unordered_set of group ids
  // Vertex and cell groups kept separate as their global ids overlap
  std::unordered_map<vtkIdType, std::set<vtkIdType>> cellIdToCellGroupIds;
  std::unordered_map<vtkIdType, std::set<vtkIdType>> cellIdToVertexGroupIds;

  // For every group
  for (size_t i = 0; i != groupInputs.size(); i++)
  {
    vtkSmartPointer<vtkUnstructuredGrid> groupData = groupInputs[i];

    // Get all the relevant cell info
    vtkCellData* cellData = groupData->GetCellData();
    vtkUnsignedCharArray* cellTypes = groupData->GetCellTypesArray();
    vtkIntArray* cellIds = vtkIntArray::SafeDownCast(cellData->GetGlobalIds());

    if (groupData->GetNumberOfCells() == 0)
    {
      continue;
    }
    auto initCellType = static_cast<VTKCellType>(cellTypes->GetValue(0));
    if (initCellType == VTK_VERTEX && !cellIds)
    {
      // The IDs we care about for node sets are point IDs.
      cellIds = vtkIntArray::SafeDownCast(groupData->GetPointData()->GetGlobalIds());
    }

    for (vtkIdType j = 0; j < groupData->GetNumberOfCells(); j++)
    {
      // Ensure homogenous cell type
      auto cellType = static_cast<VTKCellType>(cellTypes->GetValue(j));
      if (initCellType != cellType)
      {
        vtkWarningMacro(<< "Group block contains heterogenous cell types");
        return 1;
      }

      // Insert group index for this cell
      vtkIdType cellId = cellIds->GetValue(j);
      if (cellType == VTK_VERTEX)
      {
        cellIdToVertexGroupIds[cellId].insert(i);
      }
      else
      {
        cellIdToCellGroupIds[cellId].insert(i);
      }
    }
  }

  // Hashing function that produces a completely unique integer (no collision) value from set
  // ordered set so differing permutations (orderings of groups) are equivalent
  // Todo: Give every group a bit and switch to bitshift instead of pow, and/or write a struct with hasher for std::unordered_map
  // to hash with. How can I give it arguments (ie: maxSetSize)?
  auto groupIdSetToFamilyHash = [&](const vtkIdType& maxSetSize, const std::set<vtkIdType>& x) {
    vtkIdType hash = 0;
    int iter = 0;
    for (auto t : x)
    {
      hash += t * std::pow(maxSetSize, iter);
      iter++;
    }
    return hash;
  };

  // Map the hash(set of group ids) to an integer family id
  std::unordered_map<vtkIdType, int> cellGroupSetHashToFamilyId;
  cellGroupSetHashToFamilyId.clear();
  std::unordered_map<vtkIdType, int> vertexGroupSetHashToFamilyId;
  vertexGroupSetHashToFamilyId.clear();
  auto getFamilyId = [&](
                       const std::set<vtkIdType>& groupSet,
                       std::unordered_map<vtkIdType, int>& groupSetHashToFamilyId,
                       std::unordered_map<int, std::set<vtkIdType>>& familyIdToGroupSet) {
    vtkIdType groupSetHash = groupIdSetToFamilyHash(groupInputs.size(), groupSet);
    int result;
    // If it doesn't yet exist, create unique incremental id for it
    if (groupSetHashToFamilyId.count(groupSetHash) == 0)
    {
      result =
        static_cast<int>(groupSetHashToFamilyId.size()) + 1; // Group 0 reserved for default group
      groupSetHashToFamilyId[groupSetHash] = result;
      familyIdToGroupSet[result] = groupSet;
    }
    else
    {
      result = groupSetHashToFamilyId[groupSetHash];
    }
    return result;
  };

  // Create output scalars
  familyIdToVertexGroupSet.clear();
  familyIdToCellGroupSet.clear();

  // Starting with the points
  vtkPoints* points = masterInputs[0]->GetPoints();
  vtkPointData* pointData = masterInputs[0]->GetPointData();
  vtkIntArray* ptIds = vtkIntArray::SafeDownCast(pointData->GetGlobalIds());
  if (ptIds == nullptr)
  {
    vtkWarningMacro(<< "Missing point global ids");
    return 1;
  }
  // Create the scalars for this input
  vtkSmartPointer<vtkIntArray> ptFamScalars = vtkSmartPointer<vtkIntArray>::New();
  ptFamScalars->SetName(ScalarArrayName.c_str());
  ptFamScalars->SetNumberOfValues(points->GetNumberOfPoints());
  for (vtkIdType i = 0; i < points->GetNumberOfPoints(); i++)
  {
    // These ids should refer to the VTK_VERTEX cell ids
    const int ptId = ptIds->GetValue(i);
    // A cell could be unreferenced by any group, thus will not be present in the map
    if (cellIdToVertexGroupIds.count(ptId) == 0)
    {
      // In this case it shall belong to family 0, the no group family
      ptFamScalars->SetValue(i, 0);
    }
    else
    {
      const std::set<vtkIdType>& groupIds = cellIdToVertexGroupIds[ptId];
      const int familyId =
        getFamilyId(groupIds, vertexGroupSetHashToFamilyId, familyIdToVertexGroupSet);

      ptFamScalars->SetValue(i, familyId);
    }
  }

  // Then the cells
  for (size_t i = 0; i < masterInputs.size(); i++)
  {
    vtkSmartPointer<vtkUnstructuredGrid> masterData = masterInputs[i];

    vtkIntArray* cellIds = vtkIntArray::SafeDownCast(masterData->GetCellData()->GetGlobalIds());
    if (cellIds == nullptr)
    {
      vtkWarningMacro(<< "Missing cell global ids");
      return 1;
    }
    // Create the scalars for this input
    vtkSmartPointer<vtkIntArray> cellFamScalars = vtkSmartPointer<vtkIntArray>::New();
    cellFamScalars->SetName(ScalarArrayName.c_str());
    cellFamScalars->SetNumberOfValues(masterData->GetNumberOfCells());

    for (vtkIdType j = 0; j < masterData->GetNumberOfCells(); j++)
    {
      const vtkIdType cellId = cellIds->GetValue(j);

      // A cell could be unreferenced by any group, thus will not be present in the map
      if (cellIdToCellGroupIds.count(cellId) == 0)
      {
        // In this case it shall belong to family 0, the no group family
        cellFamScalars->SetValue(j, 0);
      }
      else
      {
        const std::set<vtkIdType>& groupIds = cellIdToCellGroupIds[cellId];
        const int familyId =
          getFamilyId(groupIds, cellGroupSetHashToFamilyId, familyIdToCellGroupSet);

        cellFamScalars->SetValue(j, familyId);
      }
    }

    vtkSmartPointer<vtkUnstructuredGrid> outputMasterData =
      vtkUnstructuredGrid::SafeDownCast(outputMasterBlock->GetBlock(static_cast<int>(i)));
    outputMasterData->GetCellData()->AddArray(cellFamScalars);
    outputMasterData->GetPointData()->AddArray(ptFamScalars);
  }
  return 1;
}
