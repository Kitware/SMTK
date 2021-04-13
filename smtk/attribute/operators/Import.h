//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_attribute_operators_Import_h
#define __smtk_attribute_operators_Import_h

#include "smtk/operation/ResourceManagerOperation.h"

namespace smtk
{
namespace attribute
{

/**\brief Import an attribute resource.
  */
class SMTKCORE_EXPORT Import : public smtk::operation::ResourceManagerOperation
{
public:
  smtkTypeMacro(smtk::attribute::Import);
  smtkCreateMacro(Import);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::ResourceManagerOperation);

protected:
  Result operateInternal() override;
  virtual const char* xmlDescription() const override;
};
} // namespace attribute
} // namespace smtk

#endif // __smtk_attribute_operators_Import_h
