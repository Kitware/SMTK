//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkModelEdgeOperationBase - Change properties of model entities.
// .SECTION Description
// Operation to change line resolution, the color (RGBA), user name,
// and/or visibility of a vtkModelEdge on the server.

#ifndef __vtkModelEdgeOperationBase_h
#define __vtkModelEdgeOperationBase_h

#include "cmbSystemConfig.h"
#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkModelEntityOperationBase.h"

class vtkDiscreteModel;
class vtkDiscreteModelEdge;

class VTKCMBDISCRETEMODEL_EXPORT vtkModelEdgeOperationBase : public vtkModelEntityOperationBase
{
public:
  static vtkModelEdgeOperationBase* New();
  vtkTypeMacro(vtkModelEdgeOperationBase, vtkModelEntityOperationBase);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Modify the color, user name, and/or the visibility of an object.
  virtual bool Operate(vtkDiscreteModel* Model);

  // Description:
  // Set/get the line resolution.
  vtkGetMacro(IsLineResolutionSet, int);
  vtkGetMacro(LineResolution, int);
  virtual void SetLineResolution(int resolution);

  // Description:
  // Set/get the line resolution.
  vtkDiscreteModelEdge* GetModelEdgeEntity(vtkDiscreteModel* Model);

protected:
  vtkModelEdgeOperationBase();
  virtual ~vtkModelEdgeOperationBase();

  // Description:
  // Check to see if everything is properly set for the operator.
  virtual bool AbleToOperate(vtkDiscreteModel* Model);

private:
  vtkModelEdgeOperationBase(const vtkModelEdgeOperationBase&); // Not implemented.
  void operator=(const vtkModelEdgeOperationBase&);            // Not implemented.

  int IsLineResolutionSet;
  int LineResolution;
};

#endif
