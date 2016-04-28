//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkSMTKOperator - Base class for linking vtk data to smtk operator
// .SECTION Description
// This Operator class is for linking a vtk pipeline to an smtk operator.
// For example, a vtk polydata is used as a geometry intput to an operator
// in an smtk session where the vtk data will be converted to smtk geometry.

#ifndef __smtk_vtk_SMTKOperator_h
#define __smtk_vtk_SMTKOperator_h

#include "smtk/extension/vtk/operators/Exports.h" // For export macro
#include "smtk/PublicPointerDefs.h"

#include "vtkObject.h"

class VTKSMTKOPERATORSEXT_EXPORT vtkSMTKOperator : public vtkObject
{
public:
  static vtkSMTKOperator * New();
  vtkTypeMacro(vtkSMTKOperator,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  //Description:
  //Derived class should override this methods to do vtk specific processing,
  //such as converting vtk data to smtk geometry either directly, or through
  //the m_smtkOp. The default implementation will just use m_smtkOp's corresponding
  //methods.
  virtual bool AbleToOperate();
  virtual smtk::model::OperatorResult Operate();

  //Description:
  //Set/Get related smtk operator
  virtual void SetSMTKOperator(smtk::model::OperatorPtr op);
  virtual smtk::model::OperatorPtr GetSMTKOperator();

protected:
  vtkSMTKOperator();
  virtual ~vtkSMTKOperator();

  smtk::model::WeakOperatorPtr  m_smtkOp;
private:
  vtkSMTKOperator(const vtkSMTKOperator&);  // Not implemented.
  void operator=(const vtkSMTKOperator&);  // Not implemented.
};

#endif
