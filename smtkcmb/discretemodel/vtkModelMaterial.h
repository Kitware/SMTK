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
// .NAME vtkModelMaterial -
// .SECTION Description

#ifndef __smtkcmb_vtkModelMaterial_h
#define __smtkcmb_vtkModelMaterial_h

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

