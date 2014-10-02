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
// .NAME vtkModelEdgeOperatorBase - Change properties of model entities.
// .SECTION Description
// Operator to change line resolution, the color (RGBA), user name,
// and/or visibility of a vtkModelEdge on the server.

#ifndef __vtkModelEdgeOperatorBase_h
#define __vtkModelEdgeOperatorBase_h

#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkModelEntityOperatorBase.h"
#include "cmbSystemConfig.h"

class vtkDiscreteModel;
class vtkDiscreteModelEdge;

class VTKCMBDISCRETEMODEL_EXPORT vtkModelEdgeOperatorBase : public vtkModelEntityOperatorBase
{
public:
  static vtkModelEdgeOperatorBase * New();
  vtkTypeMacro(vtkModelEdgeOperatorBase,vtkModelEntityOperatorBase);
  void PrintSelf(ostream& os, vtkIndent indent);

//BTX
  // Description:
  // Modify the color, user name, and/or the visibility of an object.
  virtual bool Operate(vtkDiscreteModel* Model);
//ETX

  // Description:
  // Set/get the line resolution.
  vtkGetMacro(IsLineResolutionSet, int);
  vtkGetMacro(LineResolution, int);
  virtual void SetLineResolution(int resolution);

//BTX
  // Description:
  // Set/get the line resolution.
  vtkDiscreteModelEdge* GetModelEdgeEntity(vtkDiscreteModel* Model);
//ETX

protected:
  vtkModelEdgeOperatorBase();
  virtual ~vtkModelEdgeOperatorBase();

  // Description:
  // Check to see if everything is properly set for the operator.
  virtual bool AbleToOperate(vtkDiscreteModel* Model);

private:
  vtkModelEdgeOperatorBase(const vtkModelEdgeOperatorBase&);  // Not implemented.
  void operator=(const vtkModelEdgeOperatorBase&);  // Not implemented.

  int IsLineResolutionSet;
  int LineResolution;
};

#endif
