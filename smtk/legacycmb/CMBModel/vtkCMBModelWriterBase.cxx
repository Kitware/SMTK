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

#include "vtkCMBModelWriterBase.h"

#include "vtkDiscreteModel.h"
#include "vtkCMBParserBase.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkCMBModelWriterV2.h"
#include "vtkCMBModelWriterV4.h"
#include "vtkCMBModelWriterV5.h"
#include "vtkFieldData.h"
#include "vtkIntArray.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"

vtkStandardNewMacro(vtkCMBModelWriterBase);

vtkCMBModelWriterBase::vtkCMBModelWriterBase()
{
  this->FileName = 0;
  this->Version = this->GetCurrentVersion();
  this->OperateSucceeded = 0;
}

vtkCMBModelWriterBase:: ~vtkCMBModelWriterBase()
{
  this->SetFileName(0);
}

void vtkCMBModelWriterBase::Operate(vtkDiscreteModelWrapper* ModelWrapper)
{
  if(!ModelWrapper)
    {
    vtkErrorMacro("Passed in a null model wrapper.");
    return;
    }
  this->Write(ModelWrapper->GetModel());
}

void vtkCMBModelWriterBase::Write(vtkDiscreteModel* model)
{
  vtkDebugMacro("Writing a CMB file.");
  this->OperateSucceeded = 0;
  if(!this->GetFileName())
    {
    vtkWarningMacro("Must set file name.");
    return;
    }

  // Note that all writers are currently derived from the
  // V2 version
  vtkCMBModelWriterV2 *writer=0;

  if(!model)
    {
    vtkErrorMacro("Passed in a null model.");
    return;
    }
  if(this->Version == 2 || this->Version == 3)
    {
    writer = vtkCMBModelWriterV2::New();
    }
  else if(this->Version == 4)
    {
    writer = vtkCMBModelWriterV4::New();
    }
   else if(this->Version == 5)
    {
    writer = vtkCMBModelWriterV5::New();
    }
  else
    {
    vtkWarningMacro("Writing version " << this->Version << " not supported.");
    }

  if (writer)
    {
    writer->SetFileName(this->GetFileName());
    this->OperateSucceeded = writer->Write(model);
    writer->Delete();
    }
  vtkDebugMacro("Finished writing a CMB file.");
  return;
}

int vtkCMBModelWriterBase::GetCurrentVersion()
{
  return 5;
}

void vtkCMBModelWriterBase::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "FileName: " << this->FileName << endl;
  os << indent << "Version: " << this->Version << endl;
  os << indent << "OperateSucceeded: " << this->OperateSucceeded << endl;
}
