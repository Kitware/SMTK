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

#include "vtkSplitEventData.h"

#include "vtkIdList.h"
#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkSplitEventData);
vtkCxxRevisionMacro(vtkSplitEventData, "");
vtkCxxSetObjectMacro(vtkSplitEventData, CreatedModelEntityIds, vtkIdList);

vtkSplitEventData::vtkSplitEventData()
{
  this->SourceEntity = NULL;
  this->CreatedModelEntityIds = NULL;
}

vtkSplitEventData::~vtkSplitEventData()
{
  // SourceEntity doesn't strictly need to be set to NULL
  // now but that may change in the future
  this->SetSourceEntity(0);
  this->SetCreatedModelEntityIds(0);
}


void vtkSplitEventData::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "SourceEntity: ";
  if(this->SourceEntity)
    {
    os << this->SourceEntity << endl;
    }
  else
    {
    os << "(NULL)\n";
    }
  os << indent << "CreatedModelEntityIds: ";
  if(this->CreatedModelEntityIds)
    {
    os << this->CreatedModelEntityIds << endl;
    }
  else
    {
    os << "(NULL)\n";
    }
}
