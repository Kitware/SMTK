//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBModelSource.h"

#include "vtkCallbackCommand.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkExecutive.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkModelGeometricEntity.h"
#include "vtkObjectFactory.h"
#include "vtkStringArray.h"

vtkStandardNewMacro(vtkCMBModelSource);

//-----------------------------------------------------------------------------
vtkCMBModelSource::vtkCMBModelSource()
{
  this->Source = 0;
  this->SetNumberOfInputPorts(0);
}

//-----------------------------------------------------------------------------
vtkCMBModelSource::~vtkCMBModelSource()
{
  this->SetSource(0);
}
//----------------------------------------------------------------------------
void vtkCMBModelSource::SetSource(vtkDiscreteModelWrapper *source)
{
  if ( this->Source == source)
    {
    return;
    }

  if ( this->Source )
    {
    this->Source = NULL;
    }

  this->Source = source;
  this->Modified();
}

//----------------------------------------------------------------------------
vtkDiscreteModelWrapper* vtkCMBModelSource::GetSource()
{
  return this->Source;
}

//-----------------------------------------------------------------------------
void vtkCMBModelSource::CopyData(vtkDiscreteModelWrapper *source)
{
  if(this->Source && source && this->Source != source)
    {
    this->Source->ShallowCopy( source );
    this->Modified();
    }
}

//----------------------------------------------------------------------------
int vtkCMBModelSource::FillOutputPortInformation(
  int vtkNotUsed(port), vtkInformation* info)
{
  info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkDiscreteModelWrapper");
  return 1;
}

//-----------------------------------------------------------------------------
vtkMTimeType vtkCMBModelSource::GetMTime()
{
  vtkMTimeType mtime = this->Superclass::GetMTime();
  if (this->Source)
    {
    vtkMTimeType stime = this->Source->GetMTime();
    if (stime > mtime)
      {
      mtime = stime;
      }
    }
  return mtime;
}

//-----------------------------------------------------------------------------
int vtkCMBModelSource::RequestDataObject(
                                   vtkInformation *vtkNotUsed(request),
                                   vtkInformationVector **vtkNotUsed(inputVector),
                                   vtkInformationVector *outputVector)
{
    // for each output
    for(int i=0; i < this->GetNumberOfOutputPorts(); ++i)
      {
      vtkInformation* outInfo = outputVector->GetInformationObject(i);
      vtkDataObject *output = outInfo->Get(vtkDataObject::DATA_OBJECT());

      if (!output || !output->IsA("vtkDiscreteModelWrapper"))
        {
        outInfo->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkDiscreteModelWrapper");
        }
      }
    return 1;
}
//-----------------------------------------------------------------------------
int vtkCMBModelSource::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{
  if(!this->Source)
    {
    return 0;
    }
  // get the ouptut
  vtkDiscreteModelWrapper *output = vtkDiscreteModelWrapper::SafeDownCast(
    outputVector->GetInformationObject(0)->Get(vtkDataObject::DATA_OBJECT()));
  if(output && output != this->Source)
    {
    output->ShallowCopy(this->Source);
    }

  return 1;
}

//-----------------------------------------------------------------------------
void vtkCMBModelSource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Source: " << this->Source << "\n";
}
