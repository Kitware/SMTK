//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkModelFaceUse.h"

#include "vtkIdList.h"
#include "vtkModel.h"
#include "vtkModelEdge.h"
#include "vtkModelEdgeUse.h"
#include "vtkModelFace.h"
#include "vtkModelItemGenericIterator.h"
#include "vtkModelItemIterator.h"
#include "vtkModelLoopUse.h"
#include "vtkModelShellUse.h"
#include "vtkModelVertex.h"
#include "vtkModelVertexUse.h"
#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkModelFaceUse);

vtkModelFaceUse::vtkModelFaceUse()
{
}

vtkModelFaceUse::~vtkModelFaceUse()
{
}

bool vtkModelFaceUse::Destroy()
{
  if (this->GetModelShellUse())
  {
    vtkErrorMacro("Trying to remove a ModelFaceUse that is still connected to a ModelShellUse.");
    return false;
  }
  this->RemoveAllAssociations(vtkModelShellUseType);

  return this->DestroyLoopUses();
}

bool vtkModelFaceUse::DestroyLoopUses()
{
  vtkModelItemIterator* iter = this->NewIterator(vtkModelLoopUseType);
  for (iter->Begin(); !iter->IsAtEnd(); iter->Next())
  {
    vtkModelLoopUse* loopUse = vtkModelLoopUse::SafeDownCast(iter->GetCurrentItem());
    if (!loopUse->Destroy())
    {
      iter->Delete();
      vtkErrorMacro("Problem destroying vtkModelLoopUse.");
      return false;
    }
  }
  iter->Delete();
  this->RemoveAllAssociations(vtkModelLoopUseType);

  return true;
}

vtkModelShellUse* vtkModelFaceUse::GetModelShellUse()
{
  vtkModelItemIterator* iter = this->NewIterator(vtkModelShellUseType);
  iter->Begin();
  vtkModelShellUse* shellUse = vtkModelShellUse::SafeDownCast(iter->GetCurrentItem());
  iter->Delete();
  return shellUse;
}

vtkModelFace* vtkModelFaceUse::GetModelFace()
{
  vtkModelItemIterator* iter = this->NewIterator(vtkModelFaceType);
  iter->Begin();
  vtkModelFace* face = vtkModelFace::SafeDownCast(iter->GetCurrentItem());
  iter->Delete();
  return face;
}

vtkModelLoopUse* vtkModelFaceUse::GetOuterLoopUse()
{
  vtkModelItemIterator* iter = this->NewIterator(vtkModelLoopUseType);
  vtkModelLoopUse* loopUse = 0;
  iter->Begin();
  if (!iter->IsAtEnd())
  {
    loopUse = vtkModelLoopUse::SafeDownCast(iter->GetCurrentItem());
  }
  iter->Delete();
  return loopUse;
}

void vtkModelFaceUse::AddLoopUse(vtkModelLoopUse* loopUse)
{
  this->AddAssociation(loopUse);
}

int vtkModelFaceUse::GetType()
{
  return vtkModelFaceUseType;
}

void vtkModelFaceUse::Serialize(vtkSerializer* ser)
{
  this->Superclass::Serialize(ser);
}

void vtkModelFaceUse::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

int vtkModelFaceUse::GetNumberOfLoopUses()
{
  vtkModelItemIterator* loops = this->NewIterator(vtkModelLoopUseType);
  int size = loops->Size();
  loops->Delete();
  return size;
}

vtkModelItemIterator* vtkModelFaceUse::NewLoopUseIterator()
{
  vtkModelItemGenericIterator* loopUses = vtkModelItemGenericIterator::New();
  vtkModelItemIterator* loops = this->NewIterator(vtkModelLoopUseType);
  for (loops->Begin(); !loops->IsAtEnd(); loops->Next())
  {
    loopUses->AddModelItem(loops->GetCurrentItem());
  }
  loops->Delete();
  return loopUses;
}
