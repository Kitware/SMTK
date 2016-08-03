//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/vtk/filter/vtkExtractLine.h"

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"

vtkStandardNewMacro(vtkExtractLine);

//----------------------------------------------------------------------------
vtkExtractLine::vtkExtractLine()
{
  this->LineId = -1;
}

//----------------------------------------------------------------------------
int vtkExtractLine::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and output
  vtkPolyData *input = vtkPolyData::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  output->SetPoints( input->GetPoints() );
  if (this->LineId < 0)
    {
    vtkWarningMacro("LineId hasn't been set, thus no line in output.");
    return 1;
    }

  if (this->LineId > input->GetNumberOfLines())
    {
    vtkWarningMacro("LineId (" << this->LineId << ") too large.  Only " <<
      input->GetNumberOfLines() << "lines in input.  No line in output.");
    return 1;
    }

  vtkIdType npts = 0, *pts = NULL;

  vtkCellArray *inputLines = input->GetLines();
  inputLines->InitTraversal();
  for (int i = 0; i <= this->LineId; i++)
    {
    inputLines->GetNextCell(npts, pts);
    }

  vtkCellArray *lines = vtkCellArray::New();
  lines->InsertNextCell(npts, pts);
  output->SetLines(lines);
  lines->Delete();

  output->GetCellData()->CopyData(input->GetCellData(),
    input->GetNumberOfVerts() + this->LineId, 0);
  return 1;
}

//----------------------------------------------------------------------------
void vtkExtractLine::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "LineId: " << this->LineId << "\n";
}
