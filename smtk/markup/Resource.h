//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_markup_Resource_h
#define smtk_markup_Resource_h

#include "smtk/graph/Resource.h"
#include "smtk/markup/Domain.h"
#include "smtk/markup/DomainFactory.h"
#include "smtk/markup/DomainMap.h"
#include "smtk/markup/Traits.h"
#include "smtk/resource/DerivedFrom.h"
#include "smtk/resource/Manager.h"

#include "smtk/markup/Exports.h"

#include "nlohmann/json.hpp"

namespace smtk
{
namespace resource
{
class CopyOptions;
}
/// Markup resource
namespace markup
{

/// A specialization of the graph resource to the markup node and arc types.
using GraphResource = smtk::graph::Resource<Traits>;

/**\brief A resource for annotating geometric models.
  *
  * The key concept this resource embodies is a set-theoretic approach
  * to referencing geometry.
  * The resource itself owns a catalog of domains, each of which provides
  * a way to enumerate its members and reference sets members (and in some
  * cases, the geometric boundaries of those members).
  *
  * There are two types of domains: discrete and parametric.
  * Discrete domains have members represented by integer identifiers
  * which live in an "ID space".
  * Parametric domains have members represented by continuous ranges
  * which live in a "parameter space".
  */
class SMTKMARKUP_EXPORT Resource
  : public smtk::resource::DerivedFrom<smtk::markup::Resource, GraphResource>
{
public:
  smtkTypeMacro(smtk::markup::Resource);
  smtkCreateMacro(smtk::resource::PersistentObject);
  smtkSuperclassMacro(smtk::resource::DerivedFrom<Resource, GraphResource>);

  Resource(const Resource&) = delete;
  ~Resource() override = default;

  /// Override methods that revise the location of the resource so
  /// we can reset resource-relative URLs.
  bool setLocation(const std::string& location) override;

  /// Override clone() to create a new markup resource using
  /// \a this resource as a template.
  std::shared_ptr<smtk::resource::Resource> clone(
    smtk::resource::CopyOptions& options) const override;

  /// Override copyInitialize() to copy domain information.
  bool copyInitialize(
    const std::shared_ptr<const smtk::resource::Resource>& source,
    smtk::resource::CopyOptions& options) override;

  // Wrap this method (instead of create()) to avoid name conflict in MSVC.
  template<typename componentT, typename... Args>
  smtk::shared_ptr<componentT> createNode(Args&&... args)
  {
    return GraphResource::create<componentT>(std::forward<Args>(args)...);
  }

  /// Return a boolean functor that classifies components according to \a query.
  std::function<bool(const smtk::resource::Component&)> queryOperation(
    const std::string& query) const override;

  /**\brief Return the resource's catalog of domains.
    *
    */
  DomainMap& domains() { return m_domains; }
  const DomainMap& domains() const { return m_domains; }

  /**\brief Return the factory used to construct markup domains.
    *
    * This factory is used to deserialize components from json objects.
    */
  static DomainFactory& domainFactory() { return s_domainFactory; }

  /**\brief Set/get the default units of length for geometric data in this resource.
    *
    * When the default is empty (which it is by default), no units are provided.
    * Otherwise, all geometric data is assumed to be in these units.
    *
    * You should not modify the length unit while geometric data exists inside
    * the resource.
    *
    * If there is no valid unit system or if \a unit is invalid
    * (i.e., not a valid unit or with dimensions other than length),
    * setLengthUnit() will fail.
    */
  std::string lengthUnit() const { return m_lengthUnit; }
  bool setLengthUnit(const std::string& unit);

protected:
  friend class Component;

  Resource(const smtk::common::UUID&, smtk::resource::Manager::Ptr manager = nullptr);
  Resource(smtk::resource::Manager::Ptr manager = nullptr);

  void initialize(); // Initialization common to all constructors.

  template<typename Modifier>
  bool modifyComponent(Component& component, const Modifier& modifier);

  DomainMap m_domains;
  static DomainFactory s_domainFactory;

  std::string m_lengthUnit;
};

template<typename Modifier>
bool Resource::modifyComponent(Component& component, const Modifier& modifier)
{
  auto& nodesById = NodeContainer::m_nodes.get<detail::IdTag>();
  auto it = nodesById.find(component.id());
  if (it != nodesById.end())
  {
    NodeContainer::m_nodes.modify(it, modifier);
    return true;
  }
  return false;
}

} // namespace markup
} // namespace smtk

// Include implicit arc implementations here since they generally need to
// invoke methods on the resource.
#include "smtk/markup/arcs/ReferencesToPrimaries.txx"

#endif // smtk_markup_Resource_h
