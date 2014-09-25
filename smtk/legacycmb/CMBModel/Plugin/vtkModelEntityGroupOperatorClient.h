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
// .NAME vtkModelEntityGroupOperatorClient - Modify a model entity group.
// .SECTION Description
// Operator to build, destroy, and/or change information about a
// vtkCMBModelEntityGroup.  As this derives
// from vtkModelEntityOperator, you can use this to also modify RGBA,
// user name and/or visibility of an entity group in addition to
// adding and/or removing model entities that are associated with
// an entity group.  If Build is used to create a new
// vtkCMBModelEntityGroup, the Id of the newly created entity group
// will be set as the Id to operate on.

#ifndef __vtkModelEntityGroupOperatorClient_h
#define __vtkModelEntityGroupOperatorClient_h

#include "vtkModelEntityGroupOperatorBase.h"
#include "cmbSystemConfig.h"

class vtkDiscreteModel;
class vtkCMBModelEntity;
class vtkCMBModelEntityGroup;
class vtkIdList;
class vtkModelEntity;
class vtkSMProxy;

class VTK_EXPORT vtkModelEntityGroupOperatorClient :
  public vtkModelEntityGroupOperatorBase
{
public:
  static vtkModelEntityGroupOperatorClient * New();
  vtkTypeMacro(vtkModelEntityGroupOperatorClient,vtkModelEntityGroupOperatorBase);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Prevent warnings about hidden base-class virtuals:
  using Superclass::Build;
  using Superclass::Destroy;
  using Superclass::Operate;

  // Description:
  // Do the operations on the specified entity group.
  virtual bool Operate(vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy);

  // Description:
  // Build the entity group on the server and client before operating
  // on it.  The function returns the UniquePersistenId of the
  // built model entity group.
  virtual vtkIdType Build(vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy);

  // Description:
  // Destroy the entity group on the server and client if possible.
  virtual bool Destroy(vtkDiscreteModel* Model, vtkSMProxy* ServerModelProxy);

protected:
  vtkModelEntityGroupOperatorClient();
  virtual ~vtkModelEntityGroupOperatorClient();

private:
  vtkModelEntityGroupOperatorClient(const vtkModelEntityGroupOperatorClient&);  // Not implemented.
  void operator=(const vtkModelEntityGroupOperatorClient&);  // Not implemented.
};

#endif
