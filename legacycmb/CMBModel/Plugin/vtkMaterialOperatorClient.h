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
// .NAME vtkMaterialOperatorClient - Modify a material.
// .SECTION Description
// Operator to build, destroy, and/or change information about a material.
// As this derives
// from vtkModelEntityOperator, you can use this to also modify RGBA,
// user name and/or visibility of a material in addition to
// adding and/or removing model entities that are associated with
// a material.  If Build is used to create a new vtkCMBMaterial,
// the Id of the newly created material will be set as the
// Id to operate on.  Note that a material can only be destroyed
// if it has no model entities associated with it (otherwise,
// those model entities would have an undefined material making
// the vtkDiscreteModel invalid).
// We may eventually need to add in the ability to set a
// warehouseid of a material here as well.


#ifndef __vtkMaterialOperatorClient_h
#define __vtkMaterialOperatorClient_h

#include "vtkMaterialOperatorBase.h"
#include "cmbSystemConfig.h"

class vtkCMBMaterial;
class vtkDiscreteModel;
class vtkIdList;
class vtkModelEntity;
class vtkModelGeometricEntity;
class vtkSMProxy;

class VTK_EXPORT vtkMaterialOperatorClient : public vtkMaterialOperatorBase
{
public:
  static vtkMaterialOperatorClient * New();
  vtkTypeMacro(vtkMaterialOperatorClient,vtkMaterialOperatorBase);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Prevent warnings about hidden base-class virtuals:
  using Superclass::Build;
  using Superclass::Destroy;
  using Superclass::Operate;

  // Description:
  // Do the operations on the specified material.
  virtual bool Operate(vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy);

  // Description:
  // Build the material on the server and client before operating
  // on it.  The function returns the UniquePersistenId of the
  // built material.
  virtual vtkIdType Build(vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy);

  // Description:
  // Destroy the material on the server and client if possible.
  virtual bool Destroy(vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy);

protected:
  vtkMaterialOperatorClient();
  virtual ~vtkMaterialOperatorClient();

private:
  vtkMaterialOperatorClient(const vtkMaterialOperatorClient&);  // Not implemented.
  void operator=(const vtkMaterialOperatorClient&);  // Not implemented.
};

#endif
