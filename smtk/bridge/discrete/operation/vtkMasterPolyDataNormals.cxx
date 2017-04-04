//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "vtkMasterPolyDataNormals.h"

#include "ModelParserHelper.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkFloatArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkIntArray.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyDataNormals.h"

vtkStandardNewMacro(vtkMasterPolyDataNormals);

namespace

{

struct Storage
{
  Storage(){}
  virtual ~Storage(){}
  virtual int value(int pos)=0;
};

template<typename vtkArrayType>
struct vtkArrayStorage : public Storage
{
  vtkArrayStorage(vtkDataArray* array)
    {
    this->Array = vtkArrayType::SafeDownCast(array);
    }
  virtual ~vtkArrayStorage()
    {
    this->Array = NULL;
    }
  virtual int value(int pos)
    {
    return static_cast<int>(this->Array->GetValue(pos));
    }
private:
  vtkArrayType* Array;
};

template<>
struct vtkArrayStorage<vtkDataArray> : public Storage
{
  vtkArrayStorage(vtkDataArray* array)
    {
    this->Array = array;
    }

  virtual ~vtkArrayStorage()
    {
    this->Array = NULL;
    }
  virtual int value(int pos)
    {
    return static_cast<int>(this->Array->GetTuple1(pos));
    }

private:
  vtkDataArray* Array;
};

struct ArrayWrapper
{
  ArrayWrapper(vtkDataArray* array)
    {
    if(array->GetDataType()==VTK_INT)
      {
      this->ArrayStorage = new vtkArrayStorage<vtkIntArray>(array);
      }
    else if(array->GetDataType()==VTK_ID_TYPE)
      {
      this->ArrayStorage = new vtkArrayStorage<vtkIdTypeArray>(array);
      }
    else
      {
      this->ArrayStorage = new vtkArrayStorage<vtkDataArray>(array);
      }
    }

  ~ArrayWrapper()
    {
    delete this->ArrayStorage;
    }


  int value(int pos)
    {
    return this->ArrayStorage->value(pos);
    }

private:
  Storage* ArrayStorage;
};

//need a way to generalize access to an
//int or id type array

}

vtkMasterPolyDataNormals::vtkMasterPolyDataNormals()
{
}

vtkMasterPolyDataNormals::~vtkMasterPolyDataNormals()
{
}

int vtkMasterPolyDataNormals::RequestData(
  vtkInformation* vtkNotUsed(request),
  vtkInformationVector** inputVector,
  vtkInformationVector* outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and ouptut
  vtkPolyData *input = vtkPolyData::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  if(input->GetNumberOfStrips() > 0)
    {
    vtkErrorMacro("Can't process input with strips! 1st decompose the strips.");
    return 0;
    }

  vtkDataArray *baseRegionIds = input->GetCellData()->GetArray(
                                 ModelParserHelper::GetShellTagName());
  if(!baseRegionIds)
    {
    vtkErrorMacro("Region CellData required!");
    return 0;
    }

  ::ArrayWrapper regionIDs(baseRegionIds);


  // input info
  vtkCellArray *inputPolys = input->GetPolys();
  int numPolys = input->GetNumberOfPolys();

  // setup the output
  output->SetPoints(input->GetPoints());
  output->GetPointData()->ShallowCopy( input->GetPointData() );
  output->GetCellData()->CopyAllocate(input->GetCellData(), input->GetNumberOfPolys());

  // polys for the output
  vtkNew<vtkCellArray> newPolys;
  newPolys->Allocate( inputPolys->GetNumberOfConnectivityEntries() );
  output->SetPolys( newPolys.GetPointer() );

  // normals for the output
  vtkNew<vtkFloatArray> normals;
  normals->SetNumberOfComponents(3);
  normals->SetNumberOfTuples(numPolys);
  normals->SetName("Normals");

  // just so we're aren't doing the BuildCells on the input
  vtkNew<vtkPolyData> workingPD;
  workingPD->ShallowCopy(input);
  // per chance there are any Verts or Lines, want to ignore them
  workingPD->SetVerts(0);
  workingPD->SetLines(0);
  workingPD->BuildCells();

  // keep track of which polys have been processed
  bool *visited = new bool[numPolys];
  memset(visited, false, numPolys*sizeof(bool));
  vtkIdType *cellIndex = new vtkIdType[numPolys];

  vtkIdType polyIndex, npts, *pts;

  // setup temporary working polydata and polys
  vtkNew<vtkPolyData> tmpPolyData;
  tmpPolyData->SetPoints( input->GetPoints() );
  vtkNew<vtkCellArray> tmpPolys;
  tmpPolys->Allocate( inputPolys->GetNumberOfConnectivityEntries() );
  tmpPolyData->SetPolys( tmpPolys.GetPointer() );

  // setup PolyDataNormals filter; assume completely closed and no
  // non-manifold edges;  Do actually expect some open surfaces (the soil), but not
  // with open portion on left side, where direction is determined
  vtkNew<vtkPolyDataNormals> pdNormals;
  pdNormals->ComputePointNormalsOff();
  pdNormals->ComputeCellNormalsOn();
  pdNormals->SplittingOff();
  pdNormals->AutoOrientNormalsOn();
  pdNormals->SetInputData( tmpPolyData.GetPointer() );

  // we stop once every poly has been visited
  vtkIdType masterPolyIndex = -1;
  while (++masterPolyIndex < numPolys)
    {
    if (visited[masterPolyIndex])
      {
      continue;
      }

    int regionId = regionIDs.value(masterPolyIndex);

    tmpPolys->Reset();
    vtkIdType numTmpCells = 0;
    // add every poly with indicated regionID to temporary polyData;
    // don't need to check any before current value of polyIndex
    for (polyIndex = masterPolyIndex; polyIndex < numPolys; polyIndex++)
      {
      // if regionId equal to current cells ID, then haven't "visited" yet
      if (regionIDs.value(polyIndex) == regionId)
        {
        workingPD->GetCellPoints(polyIndex, npts, pts);
        visited[polyIndex] = true;
        tmpPolys->InsertNextCell(npts, pts);
        cellIndex[numTmpCells++] = polyIndex;
        }
      }

    // force execution of normals filter
    pdNormals->Modified();
    pdNormals->Update();

    if (numTmpCells != pdNormals->GetOutput()->GetNumberOfPolys())
      {
      vtkErrorMacro("Lost Cells...");
      return 0;
      }

    // now pull results from filter and add to output polys
    vtkFloatArray *tmpNormals = vtkFloatArray::SafeDownCast(
      pdNormals->GetOutput()->GetCellData()->GetNormals() );
    vtkCellArray *pdNormalOutputPolys = pdNormals->GetOutput()->GetPolys();
    pdNormalOutputPolys->InitTraversal();
    float normal[3];
    for (polyIndex = 0; polyIndex < numTmpCells; polyIndex++)
      {
      pdNormalOutputPolys->GetNextCell(npts, pts);
      vtkIdType cellId = newPolys->InsertNextCell(npts, pts);
      tmpNormals->GetTypedTuple(polyIndex, normal);
      normals->SetTuple(cellId, normal);
      output->GetCellData()->CopyData(input->GetCellData(),
        cellIndex[polyIndex], cellId);
      }
    }

  //cleanup
  delete [] visited;
  delete [] cellIndex;

  output->GetCellData()->SetNormals( normals.GetPointer() );
  return 1;
}

void vtkMasterPolyDataNormals::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

