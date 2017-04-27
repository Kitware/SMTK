//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

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

#include "cmbSystemConfig.h"
#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkObject.h"

struct ADHExporterOperatorBaseInternals;
class vtkDiscreteModel;
class vtkDiscreteModelEntityGroup;

class VTKCMBDISCRETEMODEL_EXPORT vtkADHExporterOperatorBase : public vtkObject
{
public:
  static vtkADHExporterOperatorBase* New();
  vtkTypeMacro(vtkADHExporterOperatorBase, vtkObject);
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

  void AddAppliedNodalBC(int bcIndex, vtkDiscreteModelEntityGroup* bcsGroup, int bcsNodalGroupType);

  int GetNumberOfAppliedNodalBCs();
  // Description:
  // Get the bc index and unique persistent id of applied nodal bc i.
  // Returns true if successful.
  bool GetAppliedNodalBC(int i, int& bcIndex, vtkIdType& bcsGroupId, int& bcsNodalGroupType);

  // Description:
  // Remove all of the nodal groups.
  void RemoveAllAppliedNodalBCs();

  // Description:
  // Add all face for a volumetric grid or edges for a surface grid
  // in faceGroup to be written out
  // with "FCS #cellId+1 cellSide bcIndex".  bcIndex should correspond
  // to a SimBuilder BC.
  void AddAppliedElementBC(int bcIndex, vtkIdType faceGroupId);

  void AddAppliedElementBC(int bcIndex, vtkDiscreteModelEntityGroup* faceGroup);

  int GetNumberOfAppliedElementBCs();
  // Description:
  // Get the bc index and unique persistent id of applied face bc i.
  // Returns true if successful.
  bool GetAppliedElementBC(int i, int& bcIndex, vtkIdType& faceGroupId);

  // Description:
  // Remove all of the nodal groups.
  void RemoveAllAppliedElementBCs();

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

  vtkADHExporterOperatorBase(const vtkADHExporterOperatorBase&); // Not implemented.
  void operator=(const vtkADHExporterOperatorBase&);             // Not implemented.
};

#endif
