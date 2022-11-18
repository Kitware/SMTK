//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_graph_ResourceBase_h
#define smtk_graph_ResourceBase_h

#include "smtk/geometry/Manager.h"
#include "smtk/geometry/Resource.h"

#include "smtk/graph/ArcMap.h"

#include <limits>
#include <memory>
#include <string>
#include <tuple>
#include <typeindex>

namespace smtk
{
namespace graph
{

class Component;

/// A non-templated base class for graph resources. ResourceBase satisfies the
/// API of smtk::resource::Resource and isolates graph components from the traits
/// specification of smtk::graph::Resource.
class SMTKCORE_EXPORT ResourceBase
  : public smtk::resource::DerivedFrom<ResourceBase, smtk::geometry::Resource>
{
public:
  smtkTypeMacro(smtk::graph::ResourceBase);
  smtkSuperclassMacro(smtk::resource::DerivedFrom<ResourceBase, smtk::geometry::Resource>);

  virtual const ArcMap& arcs() const = 0;
  virtual ArcMap& arcs() = 0;

  /// Return a set of string-tokens holding the type-names of all accepted node types.
  virtual const std::set<smtk::string::Token>& nodeTypes() const = 0;

  /// Return a set of string-tokens holding the type-names of all accepted arc types.
  virtual const std::set<smtk::string::Token>& arcTypes() const = 0;

  /// Perform a run-time check to validate that a node is acceptable to this resource.
  virtual bool isNodeTypeAcceptable(const smtk::graph::ComponentPtr& node) = 0;

  /**\brief Insert a node, checking that nodes of its type are allowed.
    *
    * This is used by JSON deserialization code and will eventually be used
    * by python bindings to insert nodes whose type may not be known exactly
    * at runtime (or whose owning resource type may not be available at runtime).
    */
  virtual bool addNode(const smtk::graph::ComponentPtr& node)
  {
    if (this->isNodeTypeAcceptable(node))
    {
      return this->insertNode(node);
    }
    return false;
  }

  /**\brief Create a node given its type-name (and add it to the resource).
    *
    * This method is pure virtual because it needs a Traits object
    * to map the type-name to an actual type.
    */
  virtual Component* createNodeOfType(smtk::string::Token typeName) = 0;

  /**\brief Connect two nodes with a given type of arc.
    *
    * This method is pure virtual because it needs a Traits
    * object to determine validity. It exists to make python
    * bindings for graph resources (and python-defined graph
    * resources) possible.
    */
  virtual bool connect(Component* from, Component* to, smtk::string::Token arcType) = 0;

  /**\brief Disconnect all arcs from the given node.
    *
    * This method is pure virtual because it needs a Traits
    * object to iterate over all arc types for the node.
    *
    * The return value indicates whether any arcs were removed.
    */
  virtual bool disconnect(Component* node, bool explicitOnly = false) = 0;

  /**\brief Dump nodes and arcs to a file.
    *
    * If you pass an empty \a filename, then this method will dump to the console.
    *
    * By default, this will dump in the graphviz "dot" language, however "text/plain" is
    * another accepted \a mimeType.
    */
  virtual void dump(const std::string& filename, const std::string& mimeType = "text/vnd.graphviz")
    const = 0;

protected:
  /** Erase all of the nodes from the \a node storage without updating the arcs.
   *  This is an internal method used for temporary removal, modification, and
   *  re-insertion in cases where \a node data that is indexed must be changed.
   *  In that case, arcs must not be modified.
   *
   * \return the number of nodes removed. Usually this is either 0 or 1, however the
   * implementation may define removal of > 1 nodes based on criteria other than id
   * or pointer address.
   */
  virtual std::size_t eraseNodes(const smtk::graph::ComponentPtr& node) = 0;

  /** Unconditionally insert the given \a node into the container.
   *  Does not check against NodeTypes to see whether the node type is
   *  allowed; this is assumed to have already been done.
   *
   * \return whether or not the insertion was successful.
   */
  virtual bool insertNode(const smtk::graph::ComponentPtr& node) = 0;

  ResourceBase(smtk::resource::ManagerPtr manager = nullptr)
    : Superclass(manager)
  {
  }

  ResourceBase(const smtk::common::UUID& uid, smtk::resource::ManagerPtr manager = nullptr)
    : Superclass(uid, manager)
  {
  }

  ResourceBase(ResourceBase&&) noexcept = default;

  friend Component;
};

} // namespace graph
} // namespace smtk

#endif // smtk_graph_ResourceBase_h
