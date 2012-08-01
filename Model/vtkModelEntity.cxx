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
#include "vtkModelEntity.h"

#include "vtkIdList.h"
#include "vtkInformation.h"
#include "vtkInformationDoubleVectorKey.h"
#include "vtkInformationIdTypeKey.h"
#include "vtkInformationIntegerKey.h"
#include "vtkInformationObjectBaseKey.h"
#include "vtkModel.h"
#include "vtkModelItemIterator.h"
#include "vtkObjectFactory.h"
#include "vtkOpenGLProperty.h"
#include "vtkSerializer.h"


vtkInformationKeyRestrictedMacro(vtkModelEntity, COLOR, DoubleVector, 4);
vtkInformationKeyMacro(vtkModelEntity, VISIBILITY, Integer);
vtkInformationKeyMacro(vtkModelEntity, UNIQUEPERSISTENTID, IdType);
vtkInformationKeyMacro(vtkModelEntity, DISPLAY_PROPERTY, ObjectBase);
vtkInformationKeyMacro(vtkModelEntity, PICKABLE, Integer);

// Description:
// A counter to give each use a unique persistent Id that
// is different from any of the geometric model entities.
// This value is not stored in the file so it is only
// useful for a single run and probably won't be synched
// between server and client.
namespace
{
  static vtkIdType UseIdCounter = -100;
}

vtkModelEntity::vtkModelEntity()
{
  this->SetColor(-1, -1, -1, 1);
  this->Attributes = vtkInformation::New();
  this->Initialized = 0;
  this->SetUniquePersistentId(UseIdCounter--);
  this->SetVisibility(1);
}

vtkModelEntity::~vtkModelEntity()
{
  if(this->Attributes)
    {
    this->Attributes->Delete();
    this->Attributes = 0;
    }
}

void vtkModelEntity::SetColor(double r, double g, double b, double a)
{
  if(this->GetProperties()->Has(COLOR()))
    {
    double currentColor[4];
    this->GetColor(currentColor);
    if(currentColor[0]==r && currentColor[1]==g
      && currentColor[2]==b && currentColor[3]==a)
      {
      return;
      }
    }
  double color[4] = {r, g, b, a};
  this->GetProperties()->Set(COLOR(),color,4);
  this->Modified();
}

double* vtkModelEntity::GetColor()
{
  return this->GetProperties()->Get(COLOR());
}

void vtkModelEntity::GetColor(double RGBA[4])
{
  double* rgba = this->GetColor();
  for(int i=0;i<4;i++)
    {
    RGBA[i] = rgba[i];
    }
}

void vtkModelEntity::SetVisibility(int Visible)
{
  if(this->GetProperties()->Has(VISIBILITY()) &&
    Visible == this->GetProperties()->Get(VISIBILITY()))
    {
    return;
    }
  this->GetProperties()->Set(VISIBILITY(), Visible);
  this->Modified();
}

int vtkModelEntity::GetVisibility()
{
  if(this->GetProperties()->Has(VISIBILITY()))
    {
    return this->GetProperties()->Get(VISIBILITY());
    }
  return 1;
}

void vtkModelEntity::SetUniquePersistentId(vtkIdType id)
{
  vtkIdType entId = static_cast<vtkIdType>(
    this->GetProperties()->Get(UNIQUEPERSISTENTID()));
  if(entId != id)
    {
    this->GetProperties()->Set(UNIQUEPERSISTENTID(), id);
    this->Modified();
    }
}

vtkIdType vtkModelEntity::GetUniquePersistentId()
{
  return this->GetProperties()->Get(UNIQUEPERSISTENTID());
}

void vtkModelEntity::Initialize(vtkIdType UniquePersistentId)
{
  this->SetUniquePersistentId(UniquePersistentId);
  this->Initialized = 1;
}

vtkModelEntity* vtkModelEntity::GetModelEntity(vtkIdType UniquePersistentId)
{
  vtkSmartPointer<vtkIdList> types = vtkSmartPointer<vtkIdList>::New();
  this->GetItemTypesList(types);
  for(vtkIdType i=0;i<types->GetNumberOfIds();i++)
    {
    vtkModelItemIterator* iter=this->NewIterator(types->GetId(i));
    for(iter->Begin();!iter->IsAtEnd();iter->Next())
      {
      vtkModelEntity* modelEntity =
        vtkModelEntity::SafeDownCast(iter->GetCurrentItem());
      if(modelEntity &&
         modelEntity->GetUniquePersistentId() == UniquePersistentId)
        {
        iter->Delete();
        return modelEntity;
        }
      }
    iter->Delete();
    }
  return 0;
}

vtkModelEntity* vtkModelEntity::GetModelEntity(
  int Type, vtkIdType UniquePersistentId)
{
  vtkModelItemIterator* iter=this->NewIterator(Type);
  for(iter->Begin();!iter->IsAtEnd();iter->Next())
    {
    vtkModelEntity* modelEntity =
      vtkModelEntity::SafeDownCast(iter->GetCurrentItem());
    if(modelEntity &&
       modelEntity->GetUniquePersistentId() == UniquePersistentId)
      {
      iter->Delete();
      return modelEntity;
      }
    }
  iter->Delete();
  return 0;
}

void vtkModelEntity::Serialize(vtkSerializer* ser)
{
  this->Superclass::Serialize(ser);
  ser->Serialize("Attributes", this->Attributes);
}

void vtkModelEntity::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "UniquePersistentId: " << this->GetUniquePersistentId() << "\n";
}

