//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBSTLReader.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"

#include "vtkCell.h"
#include "vtkCellData.h"
#include "vtkIdTypeArray.h"
#include "vtkPolyDataConnectivityFilter.h"
#include "vtkSTLReader.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "smtk/bridge/discrete/extension/reader/vtkCMBReaderHelperFunctions.h"

namespace smtk {
  namespace bridge {
    namespace discrete {

vtkStandardNewMacro(vtkCMBSTLReader);

vtkCMBSTLReader::vtkCMBSTLReader()
{
  this->FileName = 0;
  this->SetNumberOfInputPorts(0);
}

vtkCMBSTLReader::~vtkCMBSTLReader()
{
  this->SetFileName(0);
}

int vtkCMBSTLReader::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{
  // get the info object
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the ouptut
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  std::ifstream fin(this->FileName);
  if(!fin.good())
    {
    vtkErrorMacro(<< "File " << this->FileName << " not found");
    fin.close();
    return 0;
    }
  fin.close();

  vtkSTLReader *reader = vtkSTLReader::New();
  reader->SetFileName(this->GetFileName());
  reader->ScalarTagsOn();

  //assign each region a different color scalar
  vtkPolyDataConnectivityFilter *seperateRegions = vtkPolyDataConnectivityFilter::New();
  seperateRegions->SetExtractionModeToAllRegions();
  seperateRegions->SetInputConnection( reader->GetOutputPort() );
  seperateRegions->ColorRegionsOn();
  seperateRegions->Update();

  //set the output to the seperated regions
  output->ShallowCopy(seperateRegions->GetOutput(0));
  seperateRegions->Delete();
  reader->Delete();


  vtkIdTypeArray *regions = vtkIdTypeArray::SafeDownCast( output->GetPointData()->GetScalars() );
  if ( !regions )
    {
    vtkErrorMacro("Unable to find any region.");
    return 0;
    }

  //we need to determine which region each cell is in
  //vtkPolyDataConnectivityFilter does not allow a cell to belong to more than one region
  vtkIntArray *regionArray = vtkIntArray::New();
  regionArray->SetNumberOfValues( output->GetNumberOfCells() );
  regionArray->SetName( ReaderHelperFunctions::GetShellTagName() );
  vtkIdType id=0;
  for ( vtkIdType i=0; i < output->GetNumberOfCells( ); ++i )
    {
    id = output->GetCell(i)->GetPointId(0);
    regionArray->SetValue( i, regions->GetValue( id ) );
    }

  output->GetCellData()->AddArray( regionArray );
  regionArray->Delete();


  return 1;
}

//----------------------------------------------------------------------------
int vtkCMBSTLReader::RequestInformation(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *vtkNotUsed(outputVector))
{
  if (!this->FileName)
    {
    vtkErrorMacro("FileName has to be specified!");
    return 0;
    }

  return 1;
}

void vtkCMBSTLReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "FileName: " << this->FileName << endl;
}

    } // namespace discrete
  } // namespace bridge
} // namespace smtk
