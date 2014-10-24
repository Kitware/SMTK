//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkModelMaterial.h"

#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelFace.h"
#include "vtkDiscreteModelRegion.h"
#include "vtkInformation.h"
#include "vtkInformationDoubleVectorKey.h"
#include "vtkObjectFactory.h"
#include "vtkSerializer.h"

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
  this->AddAssociation(geometricEntity);
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
  this->RemoveAssociation(geometricEntity);
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

