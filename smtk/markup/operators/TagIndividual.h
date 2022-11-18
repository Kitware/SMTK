//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_TagIndividual_h
#define smtk_markup_TagIndividual_h

#include "smtk/markup/Resource.h"
#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace markup
{

/// Create an ontology node that tags the associated objects as individual
/// instances of a class.
class SMTKMARKUP_EXPORT TagIndividual : public smtk::operation::XMLOperation
{

public:
  smtkTypeMacro(smtk::markup::TagIndividual);
  smtkCreateMacro(TagIndividual);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);

protected:
  Result operateInternal() override;
  const char* xmlDescription() const override;
};

} // namespace markup
} // namespace smtk

#endif // smtk_markup_TagIndividual_h
