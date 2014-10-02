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
// .NAME vtkADHExporterOperatorBase - Write the nodal and face boundary conditions.
// .SECTION Description
// Operator to append the nodal and face boundary conditions for
// the ADH file that is getting exported from SimBuilder.  This operator
// only appends the nodal (NDS) and face (FCS) boundary condition
// cards to the file for each node and face in the grid.  The client
// should only call this for vtkModelEntities that have an actual
// SimBuilder BC defined over them.

#ifndef __vtkADHExporterOperatorBase_h
#define __vtkADHExporterOperatorBase_h

#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkObject.h"
#include "cmbSystemConfig.h"

struct ADHExporterOperatorBaseInternals;
class vtkDiscreteModel;
class vtkDiscreteModelEntityGroup;

class VTKCMBDISCRETEMODEL_EXPORT vtkADHExporterOperatorBase : public vtkObject
{
public:
  static vtkADHExporterOperatorBase * New();
  vtkTypeMacro(vtkADHExporterOperatorBase,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get/Set the name of the file that the node and face boundary
  // condition cards will be appended to.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // Add all nodes/points in nodalGroup to be written out
  // with "NDS #pointId+1 bcIndex".  bcIndex should correspond
  // to a SimBuilder BC.
  void AddAppliedNodalBC(int bcIndex, vtkIdType bcsGroupId, int bcsNodalGroupType);
//BTX
  void AddAppliedNodalBC(int bcIndex, vtkDiscreteModelEntityGroup* bcsGroup,
   int bcsNodalGroupType);
//ETX

  int GetNumberOfAppliedNodalBCs();
  // Description:
  // Get the bc index and unique persistent id of applied nodal bc i.
  // Returns true if successful.
  bool GetAppliedNodalBC(int i, int & bcIndex, vtkIdType & bcsGroupId,
    int & bcsNodalGroupType);

  // Description:
  // Remove all of the nodal groups.
  void RemoveAllAppliedNodalBCs();

  // Description:
  // Add all face for a volumetric grid or edges for a surface grid
  // in faceGroup to be written out
  // with "FCS #cellId+1 cellSide bcIndex".  bcIndex should correspond
  // to a SimBuilder BC.
  void AddAppliedElementBC(int bcIndex, vtkIdType faceGroupId);
//BTX
  void AddAppliedElementBC(int bcIndex, vtkDiscreteModelEntityGroup* faceGroup);
//ETX

  int GetNumberOfAppliedElementBCs();
  // Description:
  // Get the bc index and unique persistent id of applied face bc i.
  // Returns true if successful.
  bool GetAppliedElementBC(int i, int & bcIndex, vtkIdType & faceGroupId);

  // Description:
  // Remove all of the nodal groups.
  void RemoveAllAppliedElementBCs();

//BTX
  // Description:
  virtual bool Operate(vtkDiscreteModel* model);

protected:
  vtkADHExporterOperatorBase();
  virtual ~vtkADHExporterOperatorBase();

  // Description:
  // Check to see if everything is properly set for the operator.
  virtual bool AbleToOperate(vtkDiscreteModel* model);

private:
  char* FileName;
  ADHExporterOperatorBaseInternals* Internal;

  vtkADHExporterOperatorBase(const vtkADHExporterOperatorBase&);  // Not implemented.
  void operator=(const vtkADHExporterOperatorBase&);  // Not implemented.
//ETX
};

#endif
