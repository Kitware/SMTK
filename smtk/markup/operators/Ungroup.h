//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_Ungroup_h
#define smtk_markup_Ungroup_h

#include "smtk/markup/Resource.h"

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace markup
{

/**\brief Ungroup a markup resource.
  */
class SMTKMARKUP_EXPORT Ungroup : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::markup::Ungroup);
  smtkCreateMacro(Ungroup);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);

protected:
  Result operateInternal() override;
  const char* xmlDescription() const override;
};

} // namespace markup
} // namespace smtk

#endif // smtk_markup_Ungroup_h
