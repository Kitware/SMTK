//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_mesh_ExtractByDihedralAngle_h
#define smtk_mesh_ExtractByDihedralAngle_h

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace mesh
{

/**\brief Extract mesh subset by accepting all neighboring facets that meet the
   selected meshset with a dihedral angle less than a given value.
  */
class SMTKCORE_EXPORT ExtractByDihedralAngle : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::mesh::ExtractByDihedralAngle);
  smtkCreateMacro(ExtractByDihedralAngle);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);

  bool ableToOperate() override;

protected:
  Result operateInternal() override;
  const char* xmlDescription() const override;
};

} //namespace mesh
} // namespace smtk

#endif // smtk_mesh_ExtractByDihedralAngle_h
