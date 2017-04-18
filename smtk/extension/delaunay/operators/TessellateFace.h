//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_extension_delaunay_TessellateFace_h
#define __smtk_extension_delaunay_TessellateFace_h

#include "smtk/extension/delaunay/Exports.h"
#include "smtk/model/Operator.h"

namespace smtk {
  namespace model {

class Session;

/**\brief Tessellate a model face using Delaunay.
  *
  * This operation updates the smtk::model::Tessellation associated with an
  * smtk::model::Face using Delaunay.
  */
class SMTKDELAUNAYEXT_EXPORT TessellateFace : public Operator
{
public:
  smtkTypeMacro(TessellateFace);
  smtkSuperclassMacro(Operator);
  smtkCreateMacro(TessellateFace);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

  virtual bool ableToOperate();

protected:
  TessellateFace();
  virtual smtk::model::OperatorResult operateInternal();
};

  } // namespace model
} // namespace smtk

#endif // __smtk_extension_delaunay_TessellateFace_h
