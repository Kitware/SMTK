/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkMergeDuplicateCells.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkMergeDuplicateCells.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkCell.h"
#include "vtkCellData.h"
#include "vtkDataArray.h"
#include "vtkIdList.h"
#include "vtkIdTypeArray.h"
#include "vtkIntArray.h"
#include "vtkMergePoints.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkCMBParserBase.h"

#include <map>
#include <vector>



//----------------------------------------------------------------------------
class vtkMergeDuplicateCells::vtkInternal
{
public:
  vtkIdType NextFaceId;
  std::map<std::pair<int,int>, vtkIdType> RegionsToFaceMap;

  vtkInternal()
    {
    this->NextFaceId = -1; // mark that this hasn't been set
    }
};

vtkStandardNewMacro(vtkMergeDuplicateCells);

//----------------------------------------------------------------------------
vtkMergeDuplicateCells::vtkMergeDuplicateCells()
{
  this->ModelFaceArrayName = 0;
  this->PassThroughPointIds = 1;
  this->ModelRegionArrayName = 0;
  this->Internal = new vtkInternal();
}

//----------------------------------------------------------------------------
vtkMergeDuplicateCells::~vtkMergeDuplicateCells()
{
  this->SetModelFaceArrayName (0);
  this->SetModelRegionArrayName(0);
  delete this->Internal;
}

//----------------------------------------------------------------------------
bool vtkMergeDuplicateCells::GetModelFaceRegions(
  vtkIdType ModelFaceId, vtkIdType & ModelRegion0Id,vtkIdType & ModelRegion1Id)
{
  // this is essentially a reverse lookup -- SLOW
  for(std::map<std::pair<int,int>, vtkIdType>::iterator it=
        this->Internal->RegionsToFaceMap.begin();
      it!=this->Internal->RegionsToFaceMap.end(); it++)
    {
    if(it->second == ModelFaceId)
      {
      ModelRegion0Id = it->first.first;
      ModelRegion1Id = it->first.second;
      return true;
      }
    }
  // for safety
  ModelRegion0Id = -1;
  ModelRegion1Id = 1;
  return false;
}


//----------------------------------------------------------------------------
int vtkMergeDuplicateCells::FillInputPortInformation(int, vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPolyData");
  return 1;
}

//----------------------------------------------------------------------------
int vtkMergeDuplicateCells::RequestData(
  vtkInformation * /*request*/,
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  vtkPolyData *input = vtkPolyData::GetData(inputVector[0], 0);
  vtkPolyData *output = vtkPolyData::GetData(outputVector, 0);
  if (!input)
    {
    vtkErrorMacro("Input not specified!");
    return 0;
    }

  vtkIntArray* RegionArray = 0;
  if (!this->ModelRegionArrayName)
    {
    vtkErrorMacro("Did not specify ModelRegionArrayName.");
    return 0;
    }
  else
    {
    RegionArray = vtkIntArray::SafeDownCast(
      input->GetCellData()->GetArray(this->ModelRegionArrayName) );
    if(RegionArray == 0)
      {
      vtkErrorMacro("Could not find region array.");
      return 0;
      }
    }
/*
  if(!this->HasDuplicateCells(input))
    {
    output->ShallowCopy(input);
    return 1;
    }
*/

  // Set the NextFaceId for model faces
  this->Internal->NextFaceId = RegionArray->GetRange(0)[1]+10;

  output->SetPoints(input->GetPoints());
  output->Allocate(input->GetNumberOfCells()/2);
  output->GetCellData()->CopyAllocate(input->GetCellData(), input->GetNumberOfPolys());

  vtkIdTypeArray* FaceArray = vtkIdTypeArray::New();
  FaceArray->SetNumberOfComponents(1);
  FaceArray->SetName(this->ModelFaceArrayName);

  std::vector<int> Checked(input->GetNumberOfCells(), 0);

  vtkIdList * ptIds = vtkIdList::New();
  vtkIdList * ptTmpIds = vtkIdList::New();
  vtkIdList * cellIds = vtkIdList::New();
  for(vtkIdType i=0;i<input->GetNumberOfCells();i++)
    {
    if(Checked[i] == 0)
      {
      input->GetCellPoints(i, ptIds);
      // Ignore degenerated cells
      if(ptIds->GetNumberOfIds()<3)
        {
        Checked[i]=1;
        continue;
        }
      input->GetCellNeighbors(i, ptIds, cellIds);
      int numCells = cellIds->GetNumberOfIds();
      if( numCells == 0)
        {
        this->InsertCell(input, i, output, RegionArray->GetValue(i), -1,
                         FaceArray);
        //this->FillCellFaceInfo(RegionArray->GetValue(i), -1,
        //                 FaceArray);
        }
      else if(numCells >= 1)
        {
        vtkIdType InsertId = i;
        vtkIdType cellId;
        bool bInserted = false;
        for(int c =0; c<numCells; c++)
          {
          cellId = cellIds->GetId(c);
          // If this duplicate cell already checked, continue
          if(Checked[cellId])
            {
            continue;
            }
          input->GetCellPoints(cellId, ptTmpIds);
          if(ptTmpIds->GetNumberOfIds() == ptIds->GetNumberOfIds())
            {
            if(!bInserted)
              {
              if(RegionArray->GetValue(cellId) > RegionArray->GetValue(i))
                {
                InsertId = cellId;
                }
              this->InsertCell(input, InsertId, output, RegionArray->GetValue(i),
                               RegionArray->GetValue(cellId), FaceArray);
              //this->FillCellFaceInfo(RegionArray->GetValue(i),
              //                 RegionArray->GetValue(cellId), FaceArray);
              bInserted = true;
              }
            Checked[cellId] = 1;
            }
          }
        }
      }
      Checked[i] = 1; // probably not needed but just to be safe
    }
  ptIds->Delete();
  ptTmpIds->Delete();
  cellIds->Delete();

  output->GetCellData()->AddArray(FaceArray);
  this->SetModelFaceRegionInfo(output);
  FaceArray->Delete();
  output->Squeeze();

  // also pass through the array of original point ids (note that
  // paraview will filter out the point data array called
  // "vtkOriginalPointIds" so you have to rename it if you want to see it
  // in paraview).
  if(this->PassThroughPointIds)
    {
    vtkDataArray* da = input->GetPointData()->GetArray("vtkOriginalPointIds");
    if(!da)
      {
      //vtkWarningMacro("Could not pass through original point ids.");
      }
    else
      {
      output->GetPointData()->AddArray(da);
      }
    }

  return 1;
}

//----------------------------------------------------------------------------
void vtkMergeDuplicateCells::InsertCell(vtkPolyData* input, vtkIdType cellId,
                                        vtkPolyData* Poly,
                                        vtkIdType Region0, vtkIdType Region1,
                                        vtkIdTypeArray* ModelFaceIdArray)
{
  vtkCell *cell = input->GetCell(cellId);
  int outputCellId = Poly->InsertNextCell(cell->GetCellType(), cell->GetPointIds());
  Poly->GetCellData()->CopyData(input->GetCellData(), cellId, outputCellId);
  if(Region1 > Region0) // take the greater value as -1 is used as a flag
    {
    int tmp = Region1;
    Region1 = Region0;
    Region0 = tmp;
    }
  vtkIdType FaceId = this->GetModelFaceId(Region0, Region1);
  ModelFaceIdArray->InsertNextTupleValue(&FaceId);
}

//----------------------------------------------------------------------------
void vtkMergeDuplicateCells::FillCellFaceInfo(
  vtkIdType Region0, vtkIdType Region1, vtkIdTypeArray* ModelFaceIdArray)
{
  if(Region1 > Region0) // take the greater value as -1 is used as a flag
    {
    int tmp = Region1;
    Region1 = Region0;
    Region0 = tmp;
    }
  vtkIdType FaceId = this->GetModelFaceId(Region0, Region1);
  ModelFaceIdArray->InsertNextTupleValue(&FaceId);
}

//----------------------------------------------------------------------------
vtkIdType vtkMergeDuplicateCells::GetModelFaceId(int Region0, int Region1)
{
  std::pair<int, int> Regions(Region0, Region1);
  std::map<std::pair<int,int>, vtkIdType>::iterator it =
    this->Internal->RegionsToFaceMap.find(Regions);
  if(it == this->Internal->RegionsToFaceMap.end())
    {
    this->Internal->RegionsToFaceMap[Regions] = this->Internal->NextFaceId;
    return this->Internal->NextFaceId++;
    }
  return it->second;
}

//----------------------------------------------------------------------------
bool vtkMergeDuplicateCells::HasDuplicateCells(vtkPolyData* polyData)
{
  if(!polyData)
    {
    return false;
    }

  vtkIdList * ptIds = vtkIdList::New();
  vtkIdList * cellIds = vtkIdList::New();
  for(vtkIdType i=0;i<polyData->GetNumberOfCells();i++)
    {
    polyData->GetCellPoints(i, ptIds);
    polyData->GetCellNeighbors(i, ptIds, cellIds);
    if(cellIds->GetNumberOfIds() == 1)
      {
      return true;
      }
    }
  ptIds->Delete();
  cellIds->Delete();

  return false;
}

//----------------------------------------------------------------------------
void vtkMergeDuplicateCells::SetModelFaceRegionInfo(vtkPolyData* Poly)
{
  vtkIdTypeArray* ModelFaceAdjacentRegionsId = vtkIdTypeArray::New();
  ModelFaceAdjacentRegionsId->SetNumberOfComponents(3);

  for(std::map<std::pair<int,int>, vtkIdType>::iterator it=
        this->Internal->RegionsToFaceMap.begin();
      it!=this->Internal->RegionsToFaceMap.end(); it++)
    {
    vtkIdType ids[3];
    ids[0] = it->second;
    ids[1] = it->first.first;
    ids[2] = it->first.second;
    ModelFaceAdjacentRegionsId->InsertNextTupleValue(ids);
    }

  ModelFaceAdjacentRegionsId->SetName(
    vtkCMBParserBase::GetModelFaceRegionsMapString());
  Poly->GetFieldData()->AddArray(ModelFaceAdjacentRegionsId);
  ModelFaceAdjacentRegionsId->Delete();
}

//----------------------------------------------------------------------------
void vtkMergeDuplicateCells::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "ModelRegionArrayName: " << this->ModelRegionArrayName << endl;
  os << indent << "ModelFaceArrayName: " << this->ModelFaceArrayName << endl;
  os << indent << "PassThroughPointIds: " << this->PassThroughPointIds << endl;
}
