//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_extension_delaunay_TessellateFaces_h
#define smtk_extension_delaunay_TessellateFaces_h

#include "smtk/extension/delaunay/Exports.h"
#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace extension
{
namespace delaunay
{

/**\brief Tessellate model faces using Delaunay.
  *
  * This operation updates the smtk::model::Tessellations associated with
  * smtk::model::Faces using Delaunay.
  */
class SMTKDELAUNAYEXT_EXPORT TessellateFaces : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::extension::delaunay::TessellateFaces);
  smtkCreateMacro(TessellateFaces);
  smtkSharedFromThisMacro(smtk::operation::Operation);

  bool ableToOperate() override;

protected:
  Result operateInternal() override;
  const char* xmlDescription() const override;
};
} // namespace delaunay
} // namespace extension
} // namespace smtk

#endif
