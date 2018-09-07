//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkAppendSolids.h"

#include "vtkAppendPolyData.h"
#include "vtkCellData.h"
#include "vtkCleanPolyData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkIntArray.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkTINStitcher.h"
#include <vtkSmartPointer.h>

#include <map>

vtkStandardNewMacro(vtkAppendSolids);

vtkAppendSolids::vtkAppendSolids()
{
  this->RegionArrayName = 0;
}

vtkAppendSolids::~vtkAppendSolids()
{
  this->SetRegionArrayName(0);
}

void vtkAppendSolids::AddInputData(vtkPolyData* input)
{
  if (input)
  {
    this->AddInputDataInternal(0, input);
  }
}

int vtkAppendSolids::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** /*inputVector*/, vtkInformationVector* /*outputVector*/)
{
  if (!this->RegionArrayName)
  {
    vtkErrorMacro("Did not specify RegionArrayName.");
    return 0;
  }

  int numberOfInputs = this->GetNumberOfInputConnections(0);
  vtkPolyData* output = vtkPolyData::SafeDownCast(this->GetOutputDataObject(0));

  // loop over inputs, assign region numbers, and append together
  vtkSmartPointer<vtkAppendPolyData> append;
  if (numberOfInputs > 1)
  {
    append = vtkSmartPointer<vtkAppendPolyData>::New();
  }

  // add each input to an append filter, and set region id (for each cell of
  // each input) so that each input can be identified after appending
  for (int i = 0; i < numberOfInputs; i++)
  {
    vtkPolyData* input = vtkPolyData::SafeDownCast(this->GetInputDataObject(0, i));

    vtkIdType numCells = input->GetNumberOfCells();

    vtkNew<vtkIntArray> regionArray;
    regionArray->SetName(this->RegionArrayName);
    regionArray->SetNumberOfValues(numCells);
    for (vtkIdType j = 0; j < numCells; j++)
    {
      regionArray->SetValue(j, i);
    }

    if (append)
    {
      vtkNew<vtkPolyData> tempPD;
      tempPD->ShallowCopy(input);

      tempPD->GetCellData()->AddArray(regionArray.GetPointer());
      append->AddInputData(tempPD.GetPointer());
    }
    else
    {
      output->ShallowCopy(input);
      output->GetCellData()->AddArray(regionArray.GetPointer());
    }
  }

  if (append)
  {
    vtkNew<vtkCleanPolyData> clean;
    clean->SetInputConnection(append->GetOutputPort());
    clean->Update();

    output->ShallowCopy(clean->GetOutput());
  }

  return 1;
}

int vtkAppendSolids::FillInputPortInformation(int port, vtkInformation* info)
{
  if (!this->Superclass::FillInputPortInformation(port, info))
  {
    return 0;
  }
  info->Set(vtkAlgorithm::INPUT_IS_REPEATABLE(), 1);
  return 1;
}

void vtkAppendSolids::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Region Array Name: " << this->RegionArrayName << endl;
}
