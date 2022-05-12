//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_Create_h
#define smtk_markup_Create_h

#include "smtk/markup/Resource.h"

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace markup
{

/**\brief Create a markup resource.
  */
class SMTKMARKUP_EXPORT Create : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::markup::Create);
  smtkCreateMacro(Create);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);

protected:
  Result operateInternal() override;
  const char* xmlDescription() const override;
  void markModifiedResources(Result&) override;
};

SMTKMARKUP_EXPORT smtk::resource::ResourcePtr create(
  const smtk::common::UUID& uid,
  const std::shared_ptr<smtk::common::Managers>& managers = nullptr);

} // namespace markup
} // namespace smtk

#endif // smtk_markup_Create_h
