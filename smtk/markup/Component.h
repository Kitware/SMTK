//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_Component_h
#define smtk_markup_Component_h

#include "smtk/markup/Exports.h"

#include "smtk/graph/ArcImplementation.h"
#include "smtk/graph/Component.h"

#include "nlohmann/json.hpp"

#include <type_traits>

namespace smtk
{
namespace resource
{
namespace json
{
class Helper;
}
} // namespace resource
namespace markup
{

template<typename ArcTraits, typename Constness, typename Outgoingness>
using ArcEndpointInterface = smtk::graph::ArcEndpointInterface<ArcTraits, Constness, Outgoingness>;
using ConstArc = smtk::graph::ConstArc;
using NonConstArc = smtk::graph::NonConstArc;
using IncomingArc = smtk::graph::IncomingArc;
using OutgoingArc = smtk::graph::OutgoingArc;

class Group;
class Label;
namespace arcs
{
struct GroupsToMembers;
struct LabelsToSubjects;
struct URLsToImportedData;
} // namespace arcs

class SMTKMARKUP_EXPORT Component : public smtk::graph::Component
{
public:
  smtkTypeMacro(smtk::markup::Component);
  smtkSuperclassMacro(smtk::graph::Component);

  using Serialize = std::true_type; // Mark this node for JSON serialization.
  using Index = std::size_t;

  template<typename... Args>
  Component(Args&&... args)
    : smtk::graph::Component(std::forward<Args>(args)...)
  {
  }
  ~Component() override;

  /// Provide an initializer for resources to call after construction.
  virtual void initialize(const nlohmann::json& data, smtk::resource::json::Helper& helper);

  /// The index is a compile-time intrinsic of the derived resource; as such, it cannot be set.
  Component::Index index() const;

  /// Return the component's name.
  std::string name() const override;

  /// Set the component's name.
  bool setName(const std::string& name);

  /// A unique integer corresponding to the component type.
  static const Component::Index type_index;

  /**\brief Return the container of groups this component belongs to.
    */
  //@{
  ArcEndpointInterface<arcs::GroupsToMembers, ConstArc, IncomingArc> groups() const;
  ArcEndpointInterface<arcs::GroupsToMembers, NonConstArc, IncomingArc> groups();
  //@}

  /**\brief Return the container of labels that reference this component.
    */
  //@{
  ArcEndpointInterface<arcs::LabelsToSubjects, ConstArc, IncomingArc> labels() const;
  ArcEndpointInterface<arcs::LabelsToSubjects, NonConstArc, IncomingArc> labels();
  //@}

  /**\brief Return the container of URLs from which this component was imported (if any).
    */
  //@{
  ArcEndpointInterface<arcs::URLsToImportedData, ConstArc, IncomingArc> importedFrom() const;
  ArcEndpointInterface<arcs::URLsToImportedData, NonConstArc, IncomingArc> importedFrom();
  //@}

protected:
  /// A functor for changing the name of a component.
  struct ModifyName;

  std::string m_name;
};

} // namespace markup
} // namespace smtk

#endif // smtk_markup_Component_h
