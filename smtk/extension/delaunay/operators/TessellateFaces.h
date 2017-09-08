//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_extension_delaunay_TessellateFaces_h
#define __smtk_extension_delaunay_TessellateFaces_h

#include "smtk/extension/delaunay/Exports.h"
#include "smtk/model/Operator.h"

namespace smtk
{
namespace mesh
{

class Session;

/**\brief Tessellate model faces using Delaunay.
  *
  * This operation updates the smtk::model::Tessellations associated with
  * smtk::model::Faces using Delaunay.
  */
class SMTKDELAUNAYEXT_EXPORT TessellateFaces : public smtk::model::Operator
{
public:
  smtkTypeMacro(TessellateFaces);
  smtkSuperclassMacro(Operator);
  smtkCreateMacro(TessellateFaces);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

  bool ableToOperate() override;

protected:
  TessellateFaces();
  smtk::model::OperatorResult operateInternal() override;
};

} // namespace model
} // namespace smtk

#endif // __smtk_extension_delaunay_TessellateFaces_h
