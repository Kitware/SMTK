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
#include "smtk/operation/XMLOperator.h"

namespace smtk
{
namespace extension
{
namespace delaunay
{

class Session;

/**\brief Tessellate model faces using Delaunay.
  *
  * This operation updates the smtk::model::Tessellations associated with
  * smtk::model::Faces using Delaunay.
  */
class SMTKDELAUNAYEXT_EXPORT TessellateFaces : public smtk::operation::XMLOperator
{
public:
  smtkTypeMacro(TessellateFaces);
  smtkSuperclassMacro(smtk::operation::XMLOperator);
  smtkCreateMacro(TessellateFaces);
  smtkSharedFromThisMacro(smtk::operation::NewOp);

  bool ableToOperate() override;

protected:
  TessellateFaces();
  Result operateInternal() override;
  const char* xmlDescription() const override;
};
}
}
}

#endif // __smtk_extension_delaunay_TessellateFaces_h
