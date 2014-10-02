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
// .NAME vtkCMBModelStateOperatorBase - Base class for the model state operator classes.
// .SECTION Description
//  This is a base class for the state operator classes,
//  vtkCMBModelStateOperator and vtkCMBModelStateOperatorClient

#ifndef __vtkCMBModelStateOperatorBase_h
#define __vtkCMBModelStateOperatorBase_h

#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkObject.h"
#include <map>
#include "cmbSystemConfig.h"

class vtkDiscreteModelWrapper;
class vtkStringArray;
class vtkIdList;
class vtkDiscreteModel;

class VTKCMBDISCRETEMODEL_EXPORT vtkCMBModelStateOperatorBase : public vtkObject
{
public:
  static vtkCMBModelStateOperatorBase *New();
  vtkTypeMacro(vtkCMBModelStateOperatorBase,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get the serialized string of the model.
  virtual vtkStringArray* GetSerializedModelString()
    {return this->SerializedModelString;}

protected:
  vtkCMBModelStateOperatorBase();
  ~vtkCMBModelStateOperatorBase();

  // Description:
  // Check to see if everything is properly set for the operator.
  virtual bool AbleToOperate(vtkDiscreteModel* Model);

  // Internal convenient ivars.
  vtkStringArray *SerializedModelString;

private:
  vtkCMBModelStateOperatorBase(const vtkCMBModelStateOperatorBase&);  // Not implemented.
  void operator=(const vtkCMBModelStateOperatorBase&);  // Not implemented.
};

#endif
