//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_attribute_operators_Associate_h
#define __smtk_attribute_operators_Associate_h

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace attribute
{

/**\brief Associate a resource to an attribute resource.

   The visualized lists of attribute component items are populated from
   associated resources.
  */
class SMTKCORE_EXPORT Associate : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::attribute::Associate);
  smtkCreateMacro(Associate);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);

protected:
  Result operateInternal() override;
  const char* xmlDescription() const override;
};
} // namespace attribute
} // namespace smtk

#endif // __smtk_attribute_operators_Associate_h
