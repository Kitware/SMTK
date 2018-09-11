//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkModelUserName.h"

#include "vtkInformation.h"
#include "vtkInformationStringKey.h"
#include "vtkModelEntity.h"
#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkModelUserName);
vtkInformationKeyMacro(vtkModelUserName, USERNAME, String);

void vtkModelUserName::SetUserName(vtkModelEntity* entity, const char* userName)
{
  entity->GetAttributes()->Set(vtkModelUserName::USERNAME(), userName);
}

const char* vtkModelUserName::GetUserName(vtkModelEntity* entity)
{
  return entity->GetAttributes()->Get(vtkModelUserName::USERNAME());
}

void vtkModelUserName::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
