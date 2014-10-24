//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

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
