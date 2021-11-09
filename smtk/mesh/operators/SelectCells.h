//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_mesh_SelectCells_h
#define smtk_mesh_SelectCells_h

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace mesh
{

/**\brief Construct a mesh selection from a mesh resource and a list of cell ids
  */
class SMTKCORE_EXPORT SelectCells : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::mesh::SelectCells);
  smtkCreateMacro(SelectCells);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);

  void generateSummary(Result&) override;

protected:
  Result operateInternal() override;
  const char* xmlDescription() const override;
};
} // namespace mesh
} // namespace smtk

#endif
