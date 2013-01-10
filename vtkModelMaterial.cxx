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
#include "vtkModelMaterial.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelFace.h"
#include "vtkDiscreteModelRegion.h"
#include "vtkInformation.h"
#include "vtkInformationDoubleVectorKey.h"
#include "vtkObjectFactory.h"
#include "vtkSerializer.h"

vtkCxxRevisionMacro(vtkModelMaterial, "");
vtkInformationKeyRestrictedMacro(vtkModelMaterial, WAREHOUSEID, DoubleVector, 2);

vtkModelMaterial* vtkModelMaterial::New()
{
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkModelMaterial");
  if(ret)
    {
    return static_cast<vtkModelMaterial*>(ret);
    }
  return new vtkModelMaterial;
}

vtkModelMaterial::vtkModelMaterial()
{
}

vtkModelMaterial::~vtkModelMaterial()
{
}

bool vtkModelMaterial::IsDestroyable()
{
  if(this->GetNumberOfModelGeometricEntities())
    {
    return false;
    }
  return true;
}

bool vtkModelMaterial::Destroy()
{
  this->RemoveAllAssociations(vtkModelVertexType);
  this->RemoveAllAssociations(vtkModelEdgeType);
  this->RemoveAllAssociations(vtkModelFaceType);
  this->RemoveAllAssociations(vtkModelRegionType);

  this->Modified();
  return true;
}

int vtkModelMaterial::GetType()
{
  return vtkModelMaterialType;
}

bool vtkModelMaterial::SetWarehouseId(double* uuid)
{
  this->GetProperties()->Set(WAREHOUSEID(),uuid,2);
  this->Modified();
  return 1;
}

double* vtkModelMaterial::GetWarehouseId()
{
  return this->GetProperties()->Get(WAREHOUSEID());
}

void vtkModelMaterial::AddModelGeometricEntity(
  vtkModelGeometricEntity* geometricEntity)
{
  // first remove the GeometricEntities association with any other material
  vtkDiscreteModelGeometricEntity* modelGeometricEntity =
    vtkDiscreteModelRegion::SafeDownCast(geometricEntity);
  if(!modelGeometricEntity)
    {
    modelGeometricEntity = vtkDiscreteModelFace::SafeDownCast(geometricEntity);
    }
  if(modelGeometricEntity)
    {
    vtkModelMaterial* previousMaterial = modelGeometricEntity->GetMaterial();
    if(previousMaterial)
      {
      previousMaterial->RemoveModelGeometricEntity(geometricEntity);
      }
    }
  this->AddAssociation(geometricEntity->GetType(), geometricEntity);
  this->Modified();
}

int vtkModelMaterial::GetNumberOfModelGeometricEntities()
{
  int number = this->GetNumberOfAssociations(vtkModelVertexType);
  number += this->GetNumberOfAssociations(vtkModelEdgeType);
  number += this->GetNumberOfAssociations(vtkModelFaceType);
  number += this->GetNumberOfAssociations(vtkModelRegionType);
  return number;
}

bool vtkModelMaterial::RemoveModelGeometricEntity(
  vtkModelGeometricEntity* geometricEntity)
{
  this->RemoveAssociation(geometricEntity->GetType(),
                          geometricEntity);
  this->Modified();
  return 1;
}

void vtkModelMaterial::Serialize(vtkSerializer* ser)
{
  this->Superclass::Serialize(ser);
}

void vtkModelMaterial::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

