//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/bridge/polygon/vtk/vtkPolygonArcProvider.h"

#include "vtkCellArray.h"
#include "vtkCompositeDataIterator.h"
#include "vtkDataObjectTreeIterator.h"
#include "vtkIdList.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"

vtkStandardNewMacro(vtkPolygonArcProvider);
//vtkCxxSetObjectMacro(vtkPolygonArcProvider,CachedOutput,vtkPolyData);

vtkPolygonArcProvider::vtkPolygonArcProvider()
{
  this->SetNumberOfInputPorts(1);
  this->SetNumberOfOutputPorts(1);
}

vtkPolygonArcProvider::~vtkPolygonArcProvider()
{
}

int vtkPolygonArcProvider::FillInputPortInformation(int, vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkMultiBlockDataSet");
  return 1;
}

int vtkPolygonArcProvider::RequestData(
  vtkInformation*, vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  vtkMultiBlockDataSet* input = vtkMultiBlockDataSet::GetData(inputVector[0], 0);
  vtkPolyData* output = vtkPolyData::GetData(outputVector, 0);

  if (!input)
  {
    vtkErrorMacro("Input not specified!");
    return 0;
  }

  if (this->BlockIndex == -1)
  {
    vtkErrorMacro("Must specify block index to extract!");
    return 0;
  }

  vtkCompositeDataIterator* iter = input->NewIterator();
  iter->SetSkipEmptyNodes(false);
  vtkDataObjectTreeIterator* treeIter = vtkDataObjectTreeIterator::SafeDownCast(iter);
  if (treeIter)
  {
    treeIter->VisitOnlyLeavesOff();
  }

  // Copy selected block over to the output.
  vtkNew<vtkPolyData> linePoly;
  linePoly->Initialize();
  int index = 0;
  for (iter->InitTraversal(); !iter->IsDoneWithTraversal(); iter->GoToNextItem(), index++)
  {
    if (index == this->BlockIndex)
    {
      vtkPolyData* block = vtkPolyData::SafeDownCast(iter->GetCurrentDataObject());
      if (block)
      {
        linePoly->ShallowCopy(block);
      }
      break;
    }
  }
  iter->Delete();

  output->Initialize();
  //see if both end nodes are the same
  // by default we always use the whole arc
  output->ShallowCopy(linePoly.GetPointer());

  return 1;
}

void vtkPolygonArcProvider::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "BlockIndex: " << this->BlockIndex << endl;
}
