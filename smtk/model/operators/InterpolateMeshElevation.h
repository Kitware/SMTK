//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_extension_vtk_InterpolateMeshElevation_h
#define __smtk_extension_vtk_InterpolateMeshElevation_h

#include "smtk/model/Operator.h"

namespace smtk
{
namespace model
{

/**\brief A class for "roughing in" a mesh elevation from interpolation points.
  */
class SMTKCORE_EXPORT InterpolateMeshElevation : public Operator
{
public:
  smtkTypeMacro(InterpolateMeshElevation);
  smtkCreateMacro(InterpolateMeshElevation);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

  virtual bool ableToOperate();

protected:
  virtual smtk::model::OperatorResult operateInternal();
};
}
}

#endif // __smtk_extension_vtk_InterpolateMeshElevation_h
