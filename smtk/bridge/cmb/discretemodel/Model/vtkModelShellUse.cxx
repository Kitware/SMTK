//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkModelShellUse.h"

#include "vtkModel.h"
#include "vtkModelFaceUse.h"
#include "vtkModelItemIterator.h"
#include "vtkModelRegion.h"
#include "vtkObjectFactory.h"


vtkModelShellUse* vtkModelShellUse::New()
{
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkModelShellUse");
  if(ret)
    {
    return static_cast<vtkModelShellUse*>(ret);
    }
  return new vtkModelShellUse;
}

vtkModelShellUse::vtkModelShellUse()
{
}

vtkModelShellUse::~vtkModelShellUse()
{
}

bool vtkModelShellUse::Destroy()
{
  this->RemoveAllAssociations(vtkModelFaceUseType);
  return true;
}

int vtkModelShellUse::GetType()
{
  return vtkModelShellUseType;
}

void vtkModelShellUse::AddModelFaceUse(vtkModelFaceUse* faceUse)
{
  faceUse->RemoveAllAssociations(this->GetType());
  this->AddAssociation(faceUse);
}

void vtkModelShellUse::RemoveModelFaceUse(vtkModelFaceUse* faceUse)
{
  this->RemoveAssociation(faceUse);
}

vtkModelItemIterator* vtkModelShellUse::NewModelFaceUseIterator()
{
  vtkModelItemIterator* iter =
    this->NewIterator(vtkModelFaceUseType);
  return iter;
}

int vtkModelShellUse::GetNumberOfModelFaceUses()
{
  return this->GetNumberOfAssociations(vtkModelFaceUseType);
}

vtkModelRegion* vtkModelShellUse::GetModelRegion()
{
  vtkModelItemIterator* iter =
    this->NewIterator(vtkModelRegionType);
  iter->Begin();
  vtkModelRegion* region =
    vtkModelRegion::SafeDownCast(iter->GetCurrentItem());
  iter->Delete();
  return region;
}

void vtkModelShellUse::Serialize(vtkSerializer* ser)
{
  this->Superclass::Serialize(ser);
}

void vtkModelShellUse::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

