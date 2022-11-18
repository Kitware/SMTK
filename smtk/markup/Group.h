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

/// A set of components owned by this collection (i.e., held by shared pointer).
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

  bool setKeys(const std::weak_ptr<smtk::markup::AssignedIds>& keys);
  const std::weak_ptr<smtk::markup::AssignedIds>& keys() const;
  std::weak_ptr<smtk::markup::AssignedIds>& keys();

  /**\brief Return the container of members of this group.
    */
  //@{
  ArcEndpointInterface<arcs::GroupsToMembers, ConstArc, OutgoingArc> members() const;
  ArcEndpointInterface<arcs::GroupsToMembers, NonConstArc, OutgoingArc> members();
  //@}

protected:
  std::weak_ptr<smtk::markup::AssignedIds> m_keys;
  bool m_ownsMembers;
};

} // namespace markup
} // namespace smtk

#endif // smtk_markup_Group_h
