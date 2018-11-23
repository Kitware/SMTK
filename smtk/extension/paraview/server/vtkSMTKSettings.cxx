//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/paraview/server/vtkSMTKSettings.h"

#include "vtkObjectFactory.h"

vtkSmartPointer<vtkSMTKSettings> vtkSMTKSettings::Instance =
  vtkSmartPointer<vtkSMTKSettings>::New();

vtkSMTKSettings* vtkSMTKSettings::New()
{
  vtkSMTKSettings* instance = vtkSMTKSettings::GetInstance();
  instance->Register(nullptr);
  return instance;
}

vtkSMTKSettings::vtkSMTKSettings()
  : HighlightOnHover(true)
  , SelectionRenderStyle(SolidSelectionStyle)
  , ResourceTreeStyle(HierarchicalStyle)
{
}

vtkSMTKSettings::~vtkSMTKSettings()
{
}

void vtkSMTKSettings::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "HighlightOnHover: " << this->HighlightOnHover << "\n";
  os << indent << "SelectionRenderStyle: " << this->SelectionRenderStyle << "\n";
  os << indent << "ResourceTreeStyle: " << this->ResourceTreeStyle << "\n";
}

vtkSMTKSettings* vtkSMTKSettings::GetInstance()
{
  if (!vtkSMTKSettings::Instance)
  {
    vtkSMTKSettings* instance = new vtkSMTKSettings();
    instance->InitializeObjectBase();
    vtkSMTKSettings::Instance.TakeReference(instance);
  }
  return vtkSMTKSettings::Instance;
}
