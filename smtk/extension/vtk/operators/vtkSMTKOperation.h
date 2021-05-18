//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkSMTKOperation - Base class for linking vtk data to smtk operator
// .SECTION Description
// This Operation class is for linking a vtk pipeline to an smtk operator.
// For example, a vtk polydata is used as a geometry intput to an operator
// in an smtk session where the vtk data will be converted to smtk geometry.

#ifndef __smtk_vtk_SMTKOperation_h
#define __smtk_vtk_SMTKOperation_h

#include "smtk/PublicPointerDefs.h"
#include "smtk/extension/vtk/operators/vtkSMTKOperationsExtModule.h" // For export macro

#include "smtk/operation/Operation.h"

#include "vtkObject.h"

class VTKSMTKOPERATIONSEXT_EXPORT vtkSMTKOperation : public vtkObject
{
public:
  static vtkSMTKOperation* New();
  vtkTypeMacro(vtkSMTKOperation, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkSMTKOperation(const vtkSMTKOperation&) = delete;
  vtkSMTKOperation& operator=(const vtkSMTKOperation&) = delete;

  //Description:
  //Derived class should override this methods to do vtk specific processing,
  //such as converting vtk data to smtk geometry either directly, or through
  //the m_smtkOp. The default implementation will just use m_smtkOp's corresponding
  //methods.
  virtual bool AbleToOperate();
  virtual smtk::operation::Operation::Result Operate();

  //Description:
  //Set/Get related smtk operator
  virtual void SetSMTKOperation(smtk::operation::Operation::Ptr op);
  virtual smtk::operation::Operation::Ptr GetSMTKOperation();

protected:
  vtkSMTKOperation();
  ~vtkSMTKOperation() override;

  std::weak_ptr<smtk::operation::Operation> m_smtkOp;
};

#endif
