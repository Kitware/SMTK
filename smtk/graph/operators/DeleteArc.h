//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_graph_DeleteArc_h
#define smtk_graph_DeleteArc_h

#include "smtk/operation/XMLOperation.h"

#include "smtk/graph/Component.h"
#include "smtk/graph/Resource.h"
#include "smtk/graph/evaluators/OwnersOf.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/GroupItem.h"
#include "smtk/attribute/ReferenceItem.h"

#include <unordered_set>

namespace smtk
{
namespace graph
{

/**\brief Delete a user-editable arc.
  *
  * Upon completion, the resource is marked as modified but no
  * components are modified.
  */
class SMTKCORE_EXPORT DeleteArc : public smtk::operation::XMLOperation
{
public:
  smtkTypeMacro(smtk::graph::DeleteArc);
  smtkSharedFromThisMacro(smtk::operation::Operation);
  smtkSuperclassMacro(smtk::operation::XMLOperation);
  smtkCreateMacro(smtk::graph::DeleteArc);

  struct EndpointTypes
  {
    std::unordered_set<smtk::string::Token> fromTypes;
    std::unordered_set<smtk::string::Token> toTypes;
  };

  // bool ableToOperate() override;

  /// Given an \a arcType and a \a resourceExemplar, find the traits
  /// object in \a resourceExemplar's arcMap() and use it to modify
  /// this operation's definitions to accept arcs of this type for
  /// deletion.
  ///
  /// If \a operationManager is provided, this method will also
  /// register this operation to the ArcDeleter operation-group
  /// for the provided \a arcType.
  static bool registerDeleter(
    smtk::string::Token arcType,
    const smtk::graph::ResourceBase* resourceExemplar,
    const std::shared_ptr<smtk::operation::Manager>& operationManager = nullptr);

  /// Register this operation to handle the given \a arcType.
  ///
  /// The \a resourceFilter indicates the subclass of `smtk::graph::ResourceBase`
  /// that the deleter should work on while the \a fromTypes and \a toTypes are
  /// type-names of node types that \a arcType is allowed to connect.
  ///
  /// If \a operationManager is provided, this method will register
  /// DeleteArc to the ArcDeleter operation-group.
  static bool registerDeleter(
    smtk::string::Token arcType,
    smtk::string::Token resourceFilter,
    const std::unordered_set<smtk::string::Token>& fromTypes,
    const std::unordered_set<smtk::string::Token>& toTypes,
    const std::shared_ptr<smtk::operation::Manager>& operationManager = nullptr);

  /// Fetch the operation's current arc type-name and the active group holding endpoints.
  bool fetchArcTypeAndEndpointsItem(
    smtk::string::Token& arcTypeName,
    smtk::attribute::GroupItem::Ptr& endpoints);

protected:
  DeleteArc();
  Result operateInternal() override;

  Specification createSpecification() override;
  const char* xmlDescription() const override;
  static bool updateSpecification(
    const Specification& spec,
    smtk::string::Token arcType,
    smtk::string::Token resourceFilter,
    const EndpointTypes& endpointTypes);
};

} // namespace graph
} // namespace smtk

#endif
