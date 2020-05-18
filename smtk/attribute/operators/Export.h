//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_attribute_operators_Export_h
#define __smtk_attribute_operators_Export_h

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace attribute
{

/**\brief Export an attribute resource.
  */
class SMTKCORE_EXPORT Export : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::attribute::Export);
  smtkCreateMacro(Export);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);

protected:
  Result operateInternal() override;
  virtual const char* xmlDescription() const override;
};
} // namespace attribute
} // namespace smtk

#endif // __smtk_attribute_operators_Export_h
