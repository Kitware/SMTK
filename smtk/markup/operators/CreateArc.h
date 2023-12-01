//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_CreateArc_h
#define smtk_markup_CreateArc_h

#include "smtk/graph/operators/CreateArc.h"

#include "smtk/markup/Component.h"
#include "smtk/markup/Resource.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ReferenceItem.h"

#include <map>
#include <set>

namespace smtk
{
namespace markup
{

/**\brief Add a new arc connecting nodes in a resource.
  *
  * This marks the endpoint components as modified.
  */
class SMTKMARKUP_EXPORT CreateArc : public smtk::graph::CreateArc
{
public:
  smtkTypeMacro(smtk::markup::CreateArc);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::graph::CreateArc);
  smtkCreateMacro(smtk::markup::CreateArc);

protected:
  CreateArc();

  const char* xmlDescription() const override;
};

} // namespace markup
} // namespace smtk

#endif
