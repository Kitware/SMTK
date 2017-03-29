//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __smtk_model_BathymetryOperator_h
#define __smtk_model_BathymetryOperator_h

#include "smtk/extension/vtk/operators/Exports.h" // For export macro
#include "smtk/model/Operator.h"
#include "vtkNew.h"

class vtkPolyData;
namespace smtk{
  namespace model{

class BathymetryHelper;

class VTKSMTKOPERATORSEXT_EXPORT BathymetryOperator : public smtk::model::Operator
{
public:
  smtkTypeMacro(BathymetryOperator);
  smtkCreateMacro(BathymetryOperator);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

  virtual bool ableToOperate();
  virtual ~BathymetryOperator();

protected:
  BathymetryOperator();
  virtual smtk::model::OperatorResult operateInternal();
  BathymetryHelper* bathyHelper;

};

  } // namespace model
} // namespace smtk

#endif // __smtk_model_BathymetryOperator_h
