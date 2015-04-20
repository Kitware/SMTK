//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkModelMaterial -
// .SECTION Description

#ifndef __smtkdiscrete_vtkModelMaterial_h
#define __smtkdiscrete_vtkModelMaterial_h

#include "vtkSMTKDiscreteModelModule.h" // For export macro

#include "vtkModelEntity.h"

class vtkInformationStringKey;
class vtkModelGeometricEntity;
class vtkModelItemIterator;

class VTKSMTKDISCRETEMODEL_EXPORT vtkModelMaterial : public vtkModelEntity
{
public:
  vtkTypeMacro(vtkModelMaterial,vtkModelEntity);
  void PrintSelf(ostream& os, vtkIndent indent);

  bool SetWarehouseId(double* uuid);
  double* GetWarehouseId();

  int GetNumberOfModelGeometricEntities();
  // take NewModelGeometricEntityIterator out for now until
  // we figure out how to do it for nonmanifold models
  //vtkModelItemIterator* NewModelGeometricEntityIterator();

  virtual int GetType();
  static vtkInformationDoubleVectorKey* WAREHOUSEID();

  // Description:
  // Reads the state of an instance from an archive OR
  // writes the state of an instance to an archive. See
  // the documentation for this class for details.
  virtual void Serialize(vtkSerializer* ser);

  // Description:
  // Add in GeometricEntity to this material.  If
  // GeometricEntity is associated with another vtkMaterial, remove
  // it from that one.
  void AddModelGeometricEntity(vtkModelGeometricEntity* geometricEntity);

protected:
  vtkModelMaterial();
  virtual ~vtkModelMaterial();
  static vtkModelMaterial* New();

  friend class vtkMaterialOperatorBase;
  // Description:
  // Remove GeometricEntity from this material.  It is assumed that it
  // will be added to another material.
  bool RemoveModelGeometricEntity(vtkModelGeometricEntity* geometricEntity);

  virtual bool IsDestroyable();
  virtual bool Destroy();

private:
  vtkModelMaterial(const vtkModelMaterial&);  // Not implemented.
  void operator=(const vtkModelMaterial&);  // Not implemented.
//BTX
  friend class vtkDiscreteModel;
//ETX
};

#endif

