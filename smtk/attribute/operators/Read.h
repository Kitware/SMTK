//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_attribute_operators_Read_h
#define smtk_attribute_operators_Read_h

#include "smtk/operation/ResourceManagerOperation.h"

namespace smtk
{
namespace attribute
{

/**\brief Read an attribute resource.
  */
class SMTKCORE_EXPORT Read : public smtk::operation::ResourceManagerOperation
{
public:
  smtkTypeMacro(smtk::attribute::Read);
  smtkCreateMacro(Read);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::ResourceManagerOperation);

protected:
  Result operateInternal() override;
  const char* xmlDescription() const override;
  void markModifiedResources(Result&) override;
};

SMTKCORE_EXPORT smtk::resource::ResourcePtr read(
  const std::string&,
  const std::shared_ptr<smtk::common::Managers>&);
} // namespace attribute
} // namespace smtk

#endif // smtk_attribute_operators_Read_h
