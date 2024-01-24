//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_EditComment_h
#define smtk_markup_EditComment_h

#include "smtk/markup/Resource.h"

#include "smtk/operation/XMLOperation.h"

namespace smtk
{
namespace markup
{

/**\brief Create a Comment node and attach it to the associated object(s)
  *       or edit the text of an existing comment.
  *
  * If a single Comment-node is associated to the operation, then
  * its text is replaced with the input text.
  *
  * If a non-Comment node (or multiple nodes of any type) are associated,
  * then a new comment node will be created and attached to the input
  * nodes).
  *
  * If the (1) input text is blank and (2) the input is marked to delete
  * empty comments – the default – and (3) a single Comment node is
  * associated to the operation, then the operation will delete the
  * comment node and any arcs connected to it.
  */
class SMTKMARKUP_EXPORT EditComment : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::markup::EditComment);
  smtkCreateMacro(EditComment);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);

protected:
  Result operateInternal() override;
  const char* xmlDescription() const override;
};

} // namespace markup
} // namespace smtk

#endif // smtk_markup_EditComment_h
