//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_graph_CreateArc_h
#define smtk_graph_CreateArc_h

#include "smtk/operation/XMLOperation.h"

#include "smtk/graph/Component.h"
#include "smtk/graph/Resource.h"
#include "smtk/graph/evaluators/OwnersOf.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ReferenceItem.h"

#include <map>
#include <set>

namespace smtk
{
namespace graph
{

/**\brief Add a new user-editable arc between nodes in a graph-based resource.
  *
  * Upon completion, all endpoints of all created arcs are marked as modified.
  */
class SMTKCORE_EXPORT CreateArc : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::graph::CreateArc);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);
  smtkCreateMacro(smtk::graph::CreateArc);

  bool ableToOperate() override;

protected:
  CreateArc();
  Result operateInternal() override;

  /// Used inside ableToOperate and operateInternal to decide on success or failure.
  bool fetchNodesAndCheckArcType(
    smtk::string::Token& arcTypeName,
    smtk::graph::Component::Ptr& from,
    smtk::graph::Component::Ptr& to);

  const char* xmlDescription() const override;
};

} // namespace graph
} // namespace smtk

#endif
