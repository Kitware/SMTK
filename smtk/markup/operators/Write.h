//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_markup_Write_h
#define smtk_markup_Write_h

#include "smtk/markup/Resource.h"

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace markup
{

class Component;

/**\brief Write a markup resource.
  */
class SMTKMARKUP_EXPORT Write : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::markup::Write);
  smtkCreateMacro(Write);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);

  bool ableToOperate() override;

protected:
  Result operateInternal() override;
  const char* xmlDescription() const override;
  void markModifiedResources(Result&) override;
  void generateSummary(Result&) override;

  bool
  writeData(const Component* dataNode, const std::string& filename, smtk::string::Token mimeType);
};

SMTKMARKUP_EXPORT bool write(
  const smtk::resource::ResourcePtr& resource,
  const std::shared_ptr<smtk::common::Managers>& managers = nullptr);

} // namespace markup
} // namespace smtk

#endif // smtk_markup_Write_h
