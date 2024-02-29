//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_Group_h
#define smtk_markup_Group_h

#include "smtk/markup/Component.h"

#include "smtk/markup/AssignedIds.h"

namespace smtk
{
namespace markup
{

/// A set of components owned by this component.
///
/// The GroupsToMembers arc enforces ownership; the members
/// each "own" the group so that the members cannot be
/// deleted without deleting the group as well.
/// You must "ungroup" the members to delete them individually.
class SMTKMARKUP_EXPORT Group : public smtk::markup::Component
{
public:
  smtkTypeMacro(smtk::markup::Group);
  smtkSuperclassMacro(smtk::markup::Component);

  template<typename... Args>
  Group(Args&&... args)
    : smtk::markup::Component(std::forward<Args>(args)...)
  {
  }

  ~Group() override;

  /// Provide an initializer for resources to call after construction.
  void initialize(const nlohmann::json& data, smtk::resource::json::Helper& helper) override;

  /**\brief Return the container of members of this group.
    */
  //@{
  ArcEndpointInterface<arcs::GroupsToMembers, ConstArc, OutgoingArc> members() const;
  ArcEndpointInterface<arcs::GroupsToMembers, NonConstArc, OutgoingArc> members();
  //@}
};

} // namespace markup
} // namespace smtk

#endif // smtk_markup_Group_h
