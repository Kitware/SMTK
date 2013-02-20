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
#include "vtkModelRegion.h"

#include "vtkIdList.h"
#include "vtkModel.h"
#include "vtkModelFace.h"
#include "vtkModelFaceUse.h"
#include "vtkModelItemGenericIterator.h"
#include <set>
#include "vtkModelShellUse.h"
#include "vtkObjectFactory.h"


//-----------------------------------------------------------------------------
vtkModelRegion::vtkModelRegion()
{
  // We don't build a model shell use yet because we don't know
  // how to yet (e.g. we don't know what model face use 
  // it is comprised of/associated with
}

//-----------------------------------------------------------------------------
vtkModelRegion::~vtkModelRegion()
{
}

//-----------------------------------------------------------------------------
int vtkModelRegion::GetType()
{
  return vtkModelRegionType;
}

//-----------------------------------------------------------------------------
bool vtkModelRegion::IsDestroyable()
{
  return 1;
}

//-----------------------------------------------------------------------------
bool vtkModelRegion::Destroy()
{
  this->GetModel()->InvokeModelGeometricEntityEvent(
    ModelGeometricEntityAboutToDestroy, this);
  vtkModelItemIterator* shellUseIter = this->NewModelShellUseIterator();
  for(shellUseIter->Begin();!shellUseIter->IsAtEnd();shellUseIter->Next())
    {
    if(!vtkModelShellUse::SafeDownCast(shellUseIter->GetCurrentItem())->Destroy())
      {
      shellUseIter->Delete();
      return 0;
      }
    }
  shellUseIter->Delete();
  this->RemoveAllAssociations(vtkModelShellUseType);
  return 1;
}

//-----------------------------------------------------------------------------
void vtkModelRegion::Initialize(vtkIdType ModelRegionId)
{
  this->Superclass::Initialize(ModelRegionId);
}

//-----------------------------------------------------------------------------
void vtkModelRegion::Initialize(int NumModelFaces, vtkModelFace** Faces, int* FaceSides,
                                vtkIdType ModelRegionId)
{
  this->Superclass::Initialize(ModelRegionId);
  BuildModelShellUse(NumModelFaces, Faces, FaceSides);
}

//-----------------------------------------------------------------------------
vtkModelItemIterator* vtkModelRegion::NewModelShellUseIterator()
{
  return this->NewIterator(vtkModelShellUseType);
}

//-----------------------------------------------------------------------------
vtkModelShellUse* vtkModelRegion::BuildModelShellUse(
  int NumModelFaces, vtkModelFace** ModelFaces, int* FaceSides)
{
  vtkModelShellUse* ShellUse = vtkModelShellUse::New();
  for(int i=0;i<NumModelFaces;i++)
    {
    ShellUse->AddModelFaceUse(ModelFaces[i]->GetModelFaceUse(FaceSides[i]));
    }
  this->AddAssociation(ShellUse);
  ShellUse->Delete();
  return ShellUse;
}

//-----------------------------------------------------------------------------
bool vtkModelRegion::DestroyModelShellUse(vtkModelShellUse* ShellUse)
{
  if(!ShellUse->Destroy())
    {
    vtkErrorMacro("Error destroying model shell use.");
    return false;
    }
  this->RemoveAssociation(ShellUse->GetType(), ShellUse);
  return true;
}

//-----------------------------------------------------------------------------
void vtkModelRegion::Serialize(vtkSerializer* ser)
{
  this->Superclass::Serialize(ser);
}

//-----------------------------------------------------------------------------
void vtkModelRegion::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//-----------------------------------------------------------------------------
vtkModelItemIterator* vtkModelRegion::NewAdjacentModelFaceIterator()
{
  vtkModelItemGenericIterator* Faces = vtkModelItemGenericIterator::New();
  vtkModelItemIterator* Shells =
    this->NewIterator(vtkModelShellUseType);
  for(Shells->Begin();!Shells->IsAtEnd();Shells->Next())
    {
    vtkModelItemIterator* FaceUses = 
      Shells->GetCurrentItem()->NewIterator(vtkModelFaceUseType);
    for(FaceUses->Begin();!FaceUses->IsAtEnd();FaceUses->Next())
      {
      Faces->AddUniqueModelItem(
        vtkModelFaceUse::SafeDownCast(FaceUses->GetCurrentItem())->
        GetModelFace());
      }
    FaceUses->Delete();
    }
  Shells->Delete();

  return Faces;
}

//-----------------------------------------------------------------------------
void  vtkModelRegion::AddShell(int , vtkModelFace** , int* )
{
  this->GetModel()->InvokeModelGeometricEntityEvent(
    ModelGeometricEntityBoundaryModified, this);
  vtkErrorMacro("Not implemented.");
}

//-----------------------------------------------------------------------------
int vtkModelRegion::GetNumberOfFaces()
{
  std::set<vtkIdType> faceIds;
  vtkIdType fid;
  vtkModelItemIterator* Shells =
    this->NewIterator(vtkModelShellUseType);
  for(Shells->Begin();!Shells->IsAtEnd();Shells->Next())
    {
    vtkModelItemIterator* FaceUses = 
      Shells->GetCurrentItem()->NewIterator(vtkModelFaceUseType);
    for(FaceUses->Begin();!FaceUses->IsAtEnd();FaceUses->Next())
      {
      fid = 
        vtkModelFaceUse::SafeDownCast(FaceUses->GetCurrentItem())->
        GetModelFace()->GetUniquePersistentId();
      faceIds.insert(fid);
      }
    FaceUses->Delete();
    }
  Shells->Delete();
  return static_cast<int>(faceIds.size());
}
//-----------------------------------------------------------------------------

int vtkModelRegion::GetNumberOfShells()
{ 
  return this->
    GetNumberOfAssociations(vtkModelShellUseType);
}
//-----------------------------------------------------------------------------
