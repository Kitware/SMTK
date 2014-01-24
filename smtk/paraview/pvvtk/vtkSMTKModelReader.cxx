/*=========================================================================

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/

#include "vtkSMTKModelReader.h"

#include "smtk/model/ImportJSON.h"
#include "smtk/model/Storage.h"
#include "smtk/paraview/vtk/vtkModelMultiBlockSource.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkNew.h"
#include "vtkSetGet.h"

using smtk::shared_ptr;
using namespace smtk::model;
using namespace smtk::util;

vtkStandardNewMacro(vtkSMTKModelReader);

vtkSMTKModelReader::vtkSMTKModelReader()
{
  this->FileName = 0;
  this->SetNumberOfInputPorts(0);
}

vtkSMTKModelReader:: ~vtkSMTKModelReader()
{
  this->SetFileName(0);
}

int vtkSMTKModelReader::RequestData(
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
  StoragePtr sm = Storage::create();

  int status = ! ImportJSON::intoModel(data.c_str(), sm);
  if (status)
    {
    vtkErrorMacro( << "Error status from ImportJSON: " << status);
    return 0;
    }

  vtkNew<vtkModelMultiBlockSource> src;
  src->SetModel(sm);
  src->Update();
  output->ShallowCopy(src->GetOutput());
  this->JSONModel = data;
  //vtkErrorMacro( << "json model (JSONModel): " << this->JSONModel.c_str());
  return 1;
}

//----------------------------------------------------------------------------
int vtkSMTKModelReader::RequestInformation(
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

void vtkSMTKModelReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "FileName: " << this->FileName << endl;
}
