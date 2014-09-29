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
// .NAME vtkCreateModelEdgesOperatorBase - Creates the model edges of a model
// .SECTION Description
// Operator to extract the model edges of a model that also consists of model regions

#ifndef __smtkcmb_vtkCreateModelEdgesOperatorBase_h
#define __smtkcmb_vtkCreateModelEdgesOperatorBase_h

#include "vtkSMTKCMBModelModule.h" // For export macro
#include "vtkObject.h"


class vtkDiscreteModel;

class VTKSMTKCMBMODEL_EXPORT vtkCreateModelEdgesOperatorBase : public vtkObject
{
public:
  static vtkCreateModelEdgesOperatorBase * New();
  vtkTypeMacro(vtkCreateModelEdgesOperatorBase,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:
  vtkCreateModelEdgesOperatorBase();
  virtual ~vtkCreateModelEdgesOperatorBase();

  // Description:
  // Check to see if everything is properly set for the operator.
  virtual bool AbleToOperate(vtkDiscreteModel* Model);

private:

  vtkCreateModelEdgesOperatorBase(const vtkCreateModelEdgesOperatorBase&);  // Not implemented.
  void operator=(const vtkCreateModelEdgesOperatorBase&);  // Not implemented.
};

#endif
