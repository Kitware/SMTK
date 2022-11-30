//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_detail_NodeContainer_h
#define smtk_markup_detail_NodeContainer_h

#include "smtk/common/UUID.h"
#include "smtk/markup/Component.h"

#include "boost/multi_index/global_fun.hpp"
#include "boost/multi_index/mem_fun.hpp"
#include "boost/multi_index/ordered_index.hpp"
#include "boost/multi_index_container.hpp"

#include <memory>
#include <string>

namespace smtk
{
namespace markup
{

// Forward-declare the base node type
class Component;

namespace detail
{

/// Unique index of markup nodes by their UUID.
struct SMTKMARKUP_EXPORT IdTag
{
};

/// Non-unique index of markup nodes by their user-assigned name.
struct SMTKMARKUP_EXPORT NameTag
{
};

/// Non-unique index of markup nodes by their type-name.
struct SMTKMARKUP_EXPORT TypeNameTag
{
};

/// Non-unique index of markup nodes by their type-index.
struct SMTKMARKUP_EXPORT TypeIndexTag
{
};

/// Extract a component's UUID for indexing.
inline const smtk::common::UUID& id(const smtk::markup::Component::Ptr& c)
{
  return c->id();
}

/// Extract a component's type-index for indexing.
inline smtk::markup::Component::Index typeIndex(const smtk::markup::Component::Ptr& c)
{
  return c->index();
}

/// Extract a component's type-name for indexing.
inline std::string typeName(const smtk::markup::Component::Ptr& c)
{
  return c->typeName();
}

/// Extract a component's user-assigned name for indexing.
inline std::string name(const smtk::markup::Component::Ptr& c)
{
  return c->name();
}

/**\brief Storage for markup graph nodes.
  */
class SMTKMARKUP_EXPORT NodeContainer
{
public:
  using Visitor = std::function<void(smtk::resource::ComponentPtr&)>;

  /// Call a visitor function on each node in the graph
  void visit(Visitor visitor) const;

  /// Find the node with a given \a uuid, or return nullptr.
  smtk::resource::ComponentPtr find(const smtk::common::UUID& uuid) const;
  smtk::resource::Component* component(const smtk::common::UUID& uuid) const;

  /// Find a node with the given name (and a type matching the result container's type).
  ///
  /// The \a ContainerType must have a `value_type` type-alias;
  /// only components that can be dynamically cast to this type
  /// will be included in search results.
  /// This method returns a container since many components may
  /// share the same name.
  ///
  /// Currently, \a ContainerType must hold raw pointers to nodes
  /// rather than weak or shared pointers to nodes. This may
  /// change in future versions.
  template<typename ContainerType>
  ContainerType findByName(const std::string& nodeName)
  {
    ContainerType result;
    const auto& nameIndexed = m_nodes.get<NameTag>();
    auto range = nameIndexed.equal_range(nodeName);
    for (auto it = range.first; it != range.second; ++it)
    {
      if (auto* node = dynamic_cast<typename ContainerType::value_type>(it->get()))
      {
        result.insert(result.end(), node);
      }
    }
    return result;
  }

  std::size_t erase(const smtk::common::UUID& uuid) { return this->eraseNodes(this->find(uuid)); }

protected:
  /**\brief Erase the given \a node from the container without updating any arcs.
    *
    * This internal method may be used for temporary removal, modification, and
    * re-insertion in cases where \a node data that is indexed must be changed.
    * In that case, arcs must not be modified.
    */
  std::size_t eraseNodes(const smtk::resource::ComponentPtr& node);

  /**\brief Unconditionally insert the given \a node into the container.
    *
    * Do not check against NodeTypes to see whether the node type is
    * allowed; this has already been done.
    */
  bool insertNode(const smtk::resource::ComponentPtr& node);

  /// The node-container typename, which specifies how to index nodes.
  using Container = boost::multi_index_container<
    std::shared_ptr<smtk::markup::Component>,
    boost::multi_index::indexed_by<
      boost::multi_index::ordered_unique< // Should be hashed
        boost::multi_index::tag<IdTag>,
        boost::multi_index::
          global_fun<const Component::Ptr&, const smtk::common::UUID&, &detail::id>>,
      boost::multi_index::ordered_non_unique<
        boost::multi_index::tag<NameTag>,
        boost::multi_index::global_fun<const Component::Ptr&, std::string, &detail::name>>,
      boost::multi_index::ordered_non_unique< // Should be hashed
        boost::multi_index::tag<TypeIndexTag>,
        boost::multi_index::
          global_fun<const Component::Ptr&, smtk::markup::Component::Index, &detail::typeIndex>>,
      boost::multi_index::ordered_non_unique< // Should be hashed
        boost::multi_index::tag<TypeNameTag>,
        boost::multi_index::global_fun<const Component::Ptr&, std::string, &detail::typeName>>>>;

  Container m_nodes;
};

} // namespace detail
} // namespace markup
} // namespace smtk

#endif // smtk_markup_detail_NodeContainer_h
