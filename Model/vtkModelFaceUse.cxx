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

#include "vtkModelFaceUse.h"

#include "vtkIdList.h"
#include "vtkModel.h"
#include "vtkModelEdge.h"
#include "vtkModelEdgeUse.h"
#include "vtkModelFace.h"
#include "vtkModelItemIterator.h"
#include "vtkModelItemGenericIterator.h"
#include "vtkModelLoopUse.h"
#include "vtkModelShellUse.h"
#include "vtkModelVertex.h"
#include "vtkModelVertexUse.h"
#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkModelFaceUse);

//-----------------------------------------------------------------------------
vtkModelFaceUse::vtkModelFaceUse()
{
}

//-----------------------------------------------------------------------------
vtkModelFaceUse::~vtkModelFaceUse()
{
}

//-----------------------------------------------------------------------------
bool vtkModelFaceUse::Destroy()
{
  if(this->GetModelShellUse())
    {
    vtkErrorMacro("Trying to remove a ModelFaceUse that is still connected to a ModelShellUse.");
    return false;
    }
  this->RemoveAllAssociations(vtkModelShellUseType);

  vtkModelItemIterator* iter = this->NewIterator(vtkModelLoopUseType);
  for(iter->Begin();!iter->IsAtEnd();iter->Next())
    {
    vtkModelLoopUse* LoopUse = 
      vtkModelLoopUse::SafeDownCast(iter->GetCurrentItem());
    if(!LoopUse->Destroy())
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

//-----------------------------------------------------------------------------
vtkModelShellUse* vtkModelFaceUse::GetModelShellUse()
{
  vtkModelItemIterator* iter = this->NewIterator(vtkModelShellUseType);
  iter->Begin();
  vtkModelShellUse* ShellUse = vtkModelShellUse::SafeDownCast(
    iter->GetCurrentItem());
  iter->Delete();
  return ShellUse;
}

//-----------------------------------------------------------------------------
vtkModelFace* vtkModelFaceUse::GetModelFace()
{
  vtkModelItemIterator* iter = this->NewIterator(vtkModelFaceType);
  iter->Begin();
  vtkModelFace* Face = vtkModelFace::SafeDownCast(iter->GetCurrentItem());
  iter->Delete();
  return Face;
}

//-----------------------------------------------------------------------------
vtkModelLoopUse* vtkModelFaceUse::GetOuterLoopUse()
{
  vtkModelItemIterator* iter = this->NewIterator(vtkModelLoopUseType);
  vtkModelLoopUse* LoopUse = 0;
  iter->Begin();
  if(!iter->IsAtEnd())
    {
    LoopUse = vtkModelLoopUse::SafeDownCast(iter->GetCurrentItem());
    }
  iter->Delete();
  return LoopUse;
}

//-----------------------------------------------------------------------------
void vtkModelFaceUse::AddLoopUse(vtkModelLoopUse* LoopUse)
{
  this->AddAssociation(LoopUse);
}

//-----------------------------------------------------------------------------
int vtkModelFaceUse::GetType()
{
  return vtkModelFaceUseType;
}

//-----------------------------------------------------------------------------
void vtkModelFaceUse::Serialize(vtkSerializer* ser)
{
  this->Superclass::Serialize(ser);
}

//-----------------------------------------------------------------------------
void vtkModelFaceUse::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//-----------------------------------------------------------------------------
int vtkModelFaceUse::GetNumberOfLoopUses()
{
  vtkModelItemIterator* loops = this->NewIterator(vtkModelLoopUseType);
  int size = loops->Size();
  loops->Delete();
  return size;
}

//-----------------------------------------------------------------------------
vtkModelItemIterator* vtkModelFaceUse::NewLoopUseIterator()
{
  vtkModelItemGenericIterator* loopUses = vtkModelItemGenericIterator::New();
  vtkModelItemIterator* loops = this->NewIterator(vtkModelLoopUseType);
  for(loops->Begin();!loops->IsAtEnd();loops->Next())
    {
    loopUses->AddModelItem(loops->GetCurrentItem());
    }
  loops->Delete();
  return loopUses;
}
//-----------------------------------------------------------------------------

