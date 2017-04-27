//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkModelRegion.h"

#include "vtkIdList.h"
#include "vtkModel.h"
#include "vtkModelFace.h"
#include "vtkModelFaceUse.h"
#include "vtkModelItemGenericIterator.h"
#include "vtkModelShellUse.h"
#include "vtkObjectFactory.h"
#include <set>

vtkModelRegion::vtkModelRegion()
{
  // We don't build a model shell use yet because we don't know
  // how to yet (e.g. we don't know what model face use
  // it is comprised of/associated with
}

vtkModelRegion::~vtkModelRegion()
{
}

int vtkModelRegion::GetType()
{
  return vtkModelRegionType;
}

bool vtkModelRegion::IsDestroyable()
{
  return 1;
}

bool vtkModelRegion::Destroy()
{
  this->GetModel()->InvokeModelGeometricEntityEvent(ModelGeometricEntityAboutToDestroy, this);
  vtkModelItemIterator* shellUseIter = this->NewModelShellUseIterator();
  for (shellUseIter->Begin(); !shellUseIter->IsAtEnd(); shellUseIter->Next())
  {
    if (!vtkModelShellUse::SafeDownCast(shellUseIter->GetCurrentItem())->Destroy())
    {
      shellUseIter->Delete();
      return 0;
    }
  }
  shellUseIter->Delete();
  this->RemoveAllAssociations(vtkModelShellUseType);
  return 1;
}

void vtkModelRegion::Initialize(vtkIdType modelRegionId)
{
  this->Superclass::Initialize(modelRegionId);
}

void vtkModelRegion::Initialize(
  int numModelFaces, vtkModelFace** faces, int* faceSides, vtkIdType modelRegionId)
{
  this->Superclass::Initialize(modelRegionId);
  this->BuildModelShellUse(numModelFaces, faces, faceSides);
}

vtkModelItemIterator* vtkModelRegion::NewModelShellUseIterator()
{
  return this->NewIterator(vtkModelShellUseType);
}

vtkModelShellUse* vtkModelRegion::BuildModelShellUse(
  int numModelFaces, vtkModelFace** modelFaces, int* faceSides)
{
  vtkModelShellUse* shellUse = vtkModelShellUse::New();
  for (int i = 0; i < numModelFaces; i++)
  {
    shellUse->AddModelFaceUse(modelFaces[i]->GetModelFaceUse(faceSides[i]));
  }
  this->AddAssociation(shellUse);
  shellUse->FastDelete();
  return shellUse;
}

bool vtkModelRegion::DestroyModelShellUse(vtkModelShellUse* shellUse)
{
  if (!shellUse->Destroy())
  {
    vtkErrorMacro("Error destroying model shell use.");
    return false;
  }
  this->RemoveAssociation(shellUse);
  return true;
}

void vtkModelRegion::Serialize(vtkSerializer* ser)
{
  this->Superclass::Serialize(ser);
}

void vtkModelRegion::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

vtkModelItemIterator* vtkModelRegion::NewAdjacentModelFaceIterator()
{
  vtkModelItemGenericIterator* faces = vtkModelItemGenericIterator::New();
  vtkModelItemIterator* shells = this->NewIterator(vtkModelShellUseType);
  for (shells->Begin(); !shells->IsAtEnd(); shells->Next())
  {
    vtkModelItemIterator* faceUses = shells->GetCurrentItem()->NewIterator(vtkModelFaceUseType);
    for (faceUses->Begin(); !faceUses->IsAtEnd(); faceUses->Next())
    {
      faces->AddUniqueModelItem(
        vtkModelFaceUse::SafeDownCast(faceUses->GetCurrentItem())->GetModelFace());
    }
    faceUses->Delete();
  }
  shells->Delete();

  return faces;
}

void vtkModelRegion::AddShell(int, vtkModelFace**, int*)
{
  this->GetModel()->InvokeModelGeometricEntityEvent(ModelGeometricEntityBoundaryModified, this);
  vtkErrorMacro("Not implemented.");
}

int vtkModelRegion::GetNumberOfFaces()
{
  std::set<vtkIdType> faceIds;
  vtkIdType fid;
  vtkModelItemIterator* shells = this->NewIterator(vtkModelShellUseType);
  for (shells->Begin(); !shells->IsAtEnd(); shells->Next())
  {
    vtkModelItemIterator* faceUses = shells->GetCurrentItem()->NewIterator(vtkModelFaceUseType);
    for (faceUses->Begin(); !faceUses->IsAtEnd(); faceUses->Next())
    {
      fid = vtkModelFaceUse::SafeDownCast(faceUses->GetCurrentItem())
              ->GetModelFace()
              ->GetUniquePersistentId();
      faceIds.insert(fid);
    }
    faceUses->Delete();
  }
  shells->Delete();
  return static_cast<int>(faceIds.size());
}

int vtkModelRegion::GetNumberOfShells()
{
  return this->GetNumberOfAssociations(vtkModelShellUseType);
}
