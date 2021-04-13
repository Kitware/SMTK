//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_attribute_operators_Dissociate_h
#define __smtk_attribute_operators_Dissociate_h

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace attribute
{

/**\brief Dissociate a resource from an attribute resource.

   The visualized lists of attribute component items are populated from
   associated resources.
  */
class SMTKCORE_EXPORT Dissociate : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::attribute::Dissociate);
  smtkCreateMacro(Dissociate);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);

protected:
  Result operateInternal() override;
  virtual const char* xmlDescription() const override;
};
} // namespace attribute
} // namespace smtk

#endif // __smtk_attribute_operators_Dissociate_h
