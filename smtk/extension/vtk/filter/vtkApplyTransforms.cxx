//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/vtk/filter/vtkApplyTransforms.h"

#include "vtkCellData.h"
#include "vtkCompositeDataIterator.h"
#include "vtkCompositeDataSet.h"
#include "vtkDataArray.h"
#include "vtkDataSetSurfaceFilter.h"
#include "vtkDoubleArray.h"
#include "vtkFieldData.h"
#include "vtkInformation.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkTransform.h"
#include "vtkTransformFilter.h"
#include "vtkUnstructuredGrid.h"

#include <array>

vtkStandardNewMacro(vtkApplyTransforms);

vtkApplyTransforms::vtkApplyTransforms()
{
  this->TransformFilter->SetTransform(this->Transform);
  this->TransformFilter->TransformAllInputVectorsOn();
}

void vtkApplyTransforms::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

int vtkApplyTransforms::FillOutputPortInformation(int port, vtkInformation* info)
{
  (void)port;
  info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkMultiBlockDataSet");
  return 1;
}

int vtkApplyTransforms::RequestData(
  vtkInformation* request,
  vtkInformationVector** inInfo,
  vtkInformationVector* outInfo)
{
  (void)request;
  auto* input = vtkCompositeDataSet::GetData(inInfo[0]);
  auto* output = vtkCompositeDataSet::GetData(outInfo);
  output->CopyStructure(input);
  auto* iter = input->NewIterator();
  iter->SkipEmptyNodesOn();
  for (iter->InitTraversal(); !iter->IsDoneWithTraversal(); iter->GoToNextItem())
  {
    auto* blockIn = input->GetDataSet(iter);
    if (!blockIn)
    {
      continue;
    }

    auto* grid = dynamic_cast<vtkUnstructuredGrid*>(blockIn);
    // If we are dealing with unstructured data, run it through a
    // surface extraction filter.
    if (grid)
    {
      this->SurfaceFilter->PassThroughPointIdsOn();
      this->SurfaceFilter->PassThroughCellIdsOn();
      this->SurfaceFilter->SetInputDataObject(0, grid);
      this->SurfaceFilter->Update();
      // We need to copy global Id information if it exists.
      vtkPolyData* result = dynamic_cast<vtkPolyData*>(this->SurfaceFilter->GetOutputDataObject(0));
      auto* globalPointIds = grid->GetPointData()->GetGlobalIds();
      auto* globalCellIds = grid->GetCellData()->GetGlobalIds();

      if (globalPointIds)
      {
        char* globalPointIdName = globalPointIds->GetName();
        auto* newGlobalIds =
          dynamic_cast<vtkDataArray*>(result->GetPointData()->GetAbstractArray(globalPointIdName));
        if (!newGlobalIds)
        {
          std::cerr << "Could not find new global Id info for extracted points named: "
                    << globalPointIdName << "\n";
        }
        else
        {
          result->GetPointData()->SetGlobalIds(newGlobalIds);
        }
      }
      blockIn = result;
      auto* blockOut = blockIn->NewInstance();
      blockOut->ShallowCopy(blockIn);
      output->SetDataSet(iter, blockOut);
      continue;
    }

    auto* blockOut = blockIn->NewInstance();
    bool haveTransform = false;

    auto* fieldDataIn = blockIn->GetAttributesAsFieldData(vtkDataObject::FIELD);
    int transformIdx = -1;
    auto* transformField =
      vtkDoubleArray::SafeDownCast(fieldDataIn->GetArray("transform", transformIdx));
    if (transformField && transformField->GetNumberOfValues() == 16)
    {
      haveTransform = true;
    }

    if (haveTransform)
    {
      this->Transform->SetMatrix(transformField->GetPointer(0));
      this->TransformFilter->SetInputDataObject(0, blockIn);
      this->TransformFilter->Update();
      blockOut->ShallowCopy(this->TransformFilter->GetOutputDataObject(0));
    }
    else
    {
      blockOut->ShallowCopy(blockIn);
    }
    output->SetDataSet(iter, blockOut);
  }

  return 1;
}
