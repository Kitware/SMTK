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

void vtkModelShellUse::AddModelFaceUse(vtkModelFaceUse* FaceUse)
{
  FaceUse->RemoveAllAssociations(this->GetType());
  this->AddAssociation(FaceUse);
}

void vtkModelShellUse::RemoveModelFaceUse(vtkModelFaceUse* FaceUse)
{
  this->RemoveAssociation(FaceUse->GetType(), FaceUse);
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
  vtkModelRegion* Region = 
    vtkModelRegion::SafeDownCast(iter->GetCurrentItem());
  iter->Delete();
  return Region;
}

void vtkModelShellUse::Serialize(vtkSerializer* ser)
{
  this->Superclass::Serialize(ser);
}

void vtkModelShellUse::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

