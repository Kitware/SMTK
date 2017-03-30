//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_WriteMesh_h
#define __smtk_model_WriteMesh_h

#include "smtk/model/Operator.h"

namespace smtk
{
namespace model
{

/**\brief A class for writing meshes to file.
  */
class SMTKCORE_EXPORT WriteMesh : public Operator
{
public:
  smtkTypeMacro(WriteMesh);
  smtkCreateMacro(WriteMesh);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

  virtual bool ableToOperate();

protected:
  virtual smtk::model::OperatorResult operateInternal();
};

} // model namespace
} // smtk namespace

#endif // __smtk_model_WriteMesh_h
