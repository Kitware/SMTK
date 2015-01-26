//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================


#include "vtkSMTKJsonModelReader.h"

#include "smtk/model/ImportJSON.h"
#include "smtk/model/Manager.h"
#include "vtkModelMultiBlockSource.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkNew.h"
#include "vtkSetGet.h"

using smtk::shared_ptr;
using namespace smtk::model;
using namespace smtk::util;


class vtkSMTKJsonModelReader::vtkInternal
{
public:
  vtkNew<vtkModelMultiBlockSource> ReaderSource;
};

vtkStandardNewMacro(vtkSMTKJsonModelReader);

vtkSMTKJsonModelReader::vtkSMTKJsonModelReader()
{
  this->FileName = 0;
  this->SetNumberOfInputPorts(0);
  this->Internal = new vtkInternal;
}

vtkSMTKJsonModelReader:: ~vtkSMTKJsonModelReader()
{
  delete this->Internal;
  this->SetFileName(0);
}

void vtkSMTKJsonModelReader::GetEntityId2BlockIdMap(std::map<std::string, unsigned int>& uuid2mid)
{
  this->Internal->ReaderSource->GetUUID2BlockIdMap(uuid2mid);
}

int vtkSMTKJsonModelReader::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outInfo)
{
  vtkDebugMacro("Reading a JSON file.");
  std::ifstream file(this->FileName);
  if (!file.good())
    {
    vtkErrorMacro( << "Could not open file \"" << this->FileName << "\".\n\n"
      << "The file should be the path to a JSON smtk model!");
    return 0;
    }

  // get the output object
  vtkMultiBlockDataSet* output = vtkMultiBlockDataSet::GetData(outInfo, 0);
  if (!output)
    {
    vtkErrorMacro("No output dataset");
    return 0;
    }

  std::string data(
    (std::istreambuf_iterator<char>(file)),
    (std::istreambuf_iterator<char>()));

//vtkErrorMacro( << "json model (data): " << data.c_str());
  ManagerPtr sm = Manager::create();

  int status = ! ImportJSON::intoModelManager(data.c_str(), sm);
  if (status)
    {
    vtkErrorMacro( << "Error status from ImportJSON: " << status);
    return 0;
    }

  this->Internal->ReaderSource->SetModelManager(sm);
  this->Internal->ReaderSource->Update();
  output->ShallowCopy(this->Internal->ReaderSource->GetOutput());

  this->JSONModel = data;
  //vtkErrorMacro( << "json model (JSONModel): " << this->JSONModel.c_str());
  return 1;
}

//----------------------------------------------------------------------------
int vtkSMTKJsonModelReader::RequestInformation(
  vtkInformation *request,
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  if (!this->FileName)
    {
    vtkErrorMacro("FileName has to be specified!");
    return 0;
    }

  return this->Superclass::RequestInformation(request, inputVector, outputVector);;
}

void vtkSMTKJsonModelReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "FileName: " << this->FileName << endl;
}
