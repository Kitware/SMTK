//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_graph_Resource_h
#define smtk_graph_Resource_h

#include "smtk/PublicPointerDefs.h"

#include "smtk/common/CompilerInformation.h"
#include "smtk/common/TypeMap.h"
#include "smtk/common/TypeTraits.h"

#include "smtk/geometry/Manager.h"
#include "smtk/geometry/Resource.h"

#include "smtk/graph/Component.h"
#include "smtk/graph/NodeSet.h"
#include "smtk/graph/evaluators/DeleteArcs.h"
#include "smtk/graph/evaluators/Dump.h"
#include "smtk/graph/filter/Grammar.h"

#include "smtk/resource/CopyOptions.h"
#include "smtk/resource/filter/Filter.h"
#include "smtk/resource/filter/ResourceActions.h"

#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <tuple>
#include <typeindex>

namespace smtk
{
/// Subsystem for modeling using nodes connected to one another by arcs.
namespace graph
{
namespace detail
{

template<typename T, typename = void>
struct has_initializer : std::false_type
{
};

template<class T, class... Args>
struct has_initializer<
  T(Args...),
  smtk::common::void_t<decltype(std::declval<T>().initialize(std::declval<Args>()...))>>
  : std::true_type
{
};

template<class T, class... Args>
typename std::enable_if<detail::has_initializer<T(Args...)>::value>::type initialize(
  T& t,
  Args&&... args)
{
  t.initialize(std::forward<Args>(args)...);
}

template<class T, class... Args>
typename std::enable_if<!detail::has_initializer<T(Args...)>::value>::type initialize(
  T& t,
  Args&&... /* args */)
{
  (void)t;
}
} // namespace detail

/**\brief A resource for conceptual modeling of geometric components.
  *
  * The smtk::graph::Resource is defined by a Traits type that defines the node
  * types and arc types of a multipartite graph. The node types must all inherit
  * from smtk::graph::Component and comprise the elements of the resource.
  * The arc types can be any type, and can represent a 1-to-1 or 1-to-many
  * relationship between node types.
  */
template<typename Traits>
class SMTK_ALWAYS_EXPORT Resource
  : public smtk::resource::DerivedFrom<Resource<Traits>, ResourceBase>
  , public detail::GraphTraits<Traits>::NodeContainer
{

public:
  using NodeContainer = typename detail::GraphTraits<Traits>::NodeContainer;
  using TypeTraits = Traits;

  template<typename NodeType>
  using is_node = smtk::tuple_contains<NodeType, typename detail::GraphTraits<Traits>::NodeTypes>;
  template<typename ArcType>
  using is_arc = smtk::tuple_contains<ArcType, typename detail::GraphTraits<Traits>::ArcTypes>;

  smtkTypedefs(smtk::graph::Resource<Traits>);

  /// Return the resource's type, which is used by the resource manager and
  /// for persistent storage.
  std::string typeName() const override
  {
    return "smtk::graph::Resource<" + smtk::common::typeName<Traits>() + ">";
  }

  smtkCreateMacro(smtk::graph::Resource<Traits>);
  smtkSuperclassMacro(smtk::resource::DerivedFrom<Resource<Traits>, ResourceBase>);
  smtkSharedFromThisMacro(smtk::resource::PersistentObject);

  /**\brief Create a node of type \a NodeType with additional constructor arguments.
    *
    * Sometimes, it is necessary to have access to a shared pointer to the
    * constructed node during initialization (especially if using the
    * RAII (Resource Acquisition Is Initialization) idiom).
    * Because shared pointers to objects cannot be used within that object's
    * constructor, this method will invoke an initializer method on the node
    * after the node is constructed (if one exists).
    * The initializer must be a method of \a NodeType named \a initialize()
    * whose arguments are a the \a parameters passed to this method.
    */
  template<typename NodeType, typename... T>
  typename std::enable_if<is_node<NodeType>::value, std::shared_ptr<NodeType>>::type create(
    T&&... parameters)
  {
    std::shared_ptr<smtk::graph::Component> created(new NodeType(this->shared_from_this()));

    auto node = std::static_pointer_cast<NodeType>(created);
    detail::initialize(*node, std::forward<T>(parameters)...);

    add(node);
    return node;
  }

  /// Add a node of type NodeType to the resource. Return true if the insertion
  /// took place.
  template<typename NodeType>
  typename std::enable_if<is_node<NodeType>::value, bool>::type add(
    const std::shared_ptr<NodeType>& node)
  {
    if (node->resource().get() != this)
    {
      throw std::invalid_argument(
        "Cannot add nodes that reference a different (or null) resource.");
    }
    return NodeContainer::insertNode(node);
  }

  /// Remove a node from the resource. Return true if the removal took place.
  template<typename NodeType>
  typename std::enable_if<is_node<NodeType>::value, bool>::type remove(
    const std::shared_ptr<NodeType>& node)
  {
    return NodeContainer::eraseNodes(node) > 0;
  }

  /// Access the arcs of the graph resource.
  const ArcMap& arcs() const override { return m_arcs; }
  ArcMap& arcs() override { return m_arcs; }

  /// Return a functor that tests a node (component) against a string query.
  std::function<bool(const smtk::resource::Component&)> queryOperation(
    const std::string& filterString) const override
  {
    return smtk::resource::filter::Filter<smtk::graph::filter::Grammar>(filterString);
  }

  /// Copy construction of resources is disallowed.
  Resource(const Resource&) noexcept = delete;

  /// Return a shared pointer to a node (component) given its UUID.
  std::shared_ptr<smtk::resource::Component> find(const smtk::common::UUID& uuid) const override
  {
    return NodeContainer::find(uuid);
  }

  /// Return a raw pointer to a node (component) given its UUID.
  smtk::resource::Component* component(const smtk::common::UUID& uuid) const override
  {
    return NodeContainer::component(uuid);
  }

  /// Visit all the components (nodes
  void visit(std::function<void(const smtk::resource::ComponentPtr&)>& v) const override
  {
    NodeContainer::visit(v);
  }

  /**\brief Invoke a \a Functor on each arc-type's implementation.
    *
    * The functor should have a templated parenthesis operator whose template
    * parameter is the arc implementation object type, which determines the
    * the functor's first argument type.
    * This type varies for each ArcTraits type in TypeTraits::ArcTypes and
    * will be `ArcImplementation<ArcTraits>*`.
    *
    * The following arguments the functor's parenthesis-operator takes are
    * \a args forwarded from this method.
    *
    * This method allows you to traverse all types of arcs present in the
    * resource for inspection or modification.
    */
  //@{
  // const version
  template<typename Functor, typename... Args>
  void evaluateArcs(Args&&... args) const
  {
    Functor::begin(this, std::forward<Args>(args)...);
    m_arcs.invoke<typename detail::GraphTraits<Traits>::ArcTypes, Functor>(
      this, std::forward<Args>(args)...);
    // Now, for arc-map entries not present in the ArcTypes tuple,
    // invoke the Functor on the runtime arc-object base.
    m_arcs.invokeRuntime<Functor>(this, std::forward<Args>(args)...);
    Functor::end(this, std::forward<Args>(args)...);
  }

  // non-const version
  template<typename Functor, typename... Args>
  void evaluateArcs(Args&&... args)
  {
    Functor::begin(this, std::forward<Args>(args)...);
    m_arcs.invoke<typename detail::GraphTraits<Traits>::ArcTypes, Functor>(
      this, std::forward<Args>(args)...);
    // Now, for arc-map entries not present in the ArcTypes tuple,
    // invoke the Functor on the runtime arc-object base.
    m_arcs.invokeRuntime<Functor>(this, std::forward<Args>(args)...);
    Functor::end(this, std::forward<Args>(args)...);
  }
  //@}

  /// Return the set of node types accepted by this resource.
  const std::set<smtk::string::Token>& nodeTypes() const override { return m_nodeTypes; }

  /// Return the set of arc types accepted by this resource.
  const std::set<smtk::string::Token>& arcTypes() const override { return m_arcs.types(); }

  /// Override the base resource to store and retrieve a template type.
  smtk::string::Token templateType() const override { return m_templateType; }
  bool setTemplateType(const smtk::string::Token& templateType) override;

  /// Override the base resource to store and retrieve a template version number.
  std::size_t templateVersion() const override { return m_templateVersion; }
  bool setTemplateVersion(std::size_t templateVersion) override;

  /// Implement clone() to make a copy of a graph resource.
  ///
  /// The output copy will not have any nodes or arcs but will have any run-time arc types
  /// registered to match the originating resource if \a options has copyTemplateData()
  /// set to true.
  ///
  /// Note that unless concrete subclasses override this method, this will return
  /// a null pointer if no resource manager exists on \a this.
  /// This graph resource templated on a Traits object is typically not identical
  /// to a full implementation of a new graph-based resource. This implementation
  /// has no way to create a duplicate resource of the same type without the resource
  /// manager.
  ///
  /// If you choose to override this method, you may call prepareClone() described
  /// below with a bare resource to populate.
  std::shared_ptr<smtk::resource::Resource> clone(
    smtk::resource::CopyOptions& options) const override;

  /// Copy graph arc-types and other data that is part of the schema of a graph
  /// resource (rather than the content of the resource).
  ///
  /// This method is called by clone() if a \a result resource can be created.
  /// If your subclass of the graph resource overrides clone(), you are expected
  /// to call this method from within your override (or perform the equivalent
  /// initialization yourself).
  virtual void prepareClone(
    smtk::resource::CopyOptions& options,
    const std::shared_ptr<smtk::graph::ResourceBase>& result) const;

  /// Implement copyInitialize() to copy arcs and nodes from a non-empty resource of the same type.
  bool copyInitialize(
    const std::shared_ptr<const smtk::resource::Resource>& source,
    smtk::resource::CopyOptions& options) override;

  /// Implement copyFinalize() to copy any external links from a non-empty resource of
  /// the same type.
  bool copyFinalize(
    const std::shared_ptr<const smtk::resource::Resource>& source,
    smtk::resource::CopyOptions& options) override;

protected:
  /// A functor to create smtk::string::Tokens of accepted node types.
  template<typename ResourceType>
  struct CreateNodeOfType
  {
    CreateNodeOfType(ResourceType* self, smtk::string::Token typeName)
      : m_self(self)
      , m_typeName(typeName)
    {
      if (!m_self || m_typeName.data().empty())
      {
        throw std::invalid_argument("Invalid resource or empty node type-name.");
      }
    }

    template<typename T>
    void evaluate(std::size_t ii)
    {
      (void)ii;
      smtk::string::Token typeName = smtk::common::typeName<T>();
      if (typeName == m_typeName && !m_node)
      {
        m_node = m_self->template create<T>();
      }
    }

    const std::shared_ptr<Component>& createdNode() const { return m_node; }

    ResourceType* m_self;
    std::shared_ptr<Component> m_node;
    smtk::string::Token m_typeName;
  };

public:
  /// Implement the generic (untyped) graph API for inserting nodes.
  Component* createNodeOfType(smtk::string::Token nodeTypeName) override
  {
    if (m_nodeTypes.find(nodeTypeName) == m_nodeTypes.end())
    {
      return nullptr;
    }
    CreateNodeOfType<SelfType> creator(this, nodeTypeName);
    smtk::tuple_evaluate<typename Traits::NodeTypes>(creator);
    return creator.createdNode().get();
  }

  /// Implement the generic (untyped) graph API for inserting arcs.
  ///
  /// The \a from and \a to nodes are checked to ensure they are owned by
  /// this resource and the proper type for this resource. This method returns
  /// true if the arc was inserted and false if it already existed or was
  /// inappropriate.
  ///
  /// This method is considered "untyped" because the types of both the
  /// endpoint nodes and arc are unknown at compile-time. Type safety is
  /// ensured by run-time checks.
  bool connect(Component* from, Component* to, smtk::string::Token arcType) override
  {
    bool result = from->outgoing(arcType).connect(to);
    return result;
  }

  /// Remove all arcs (or only explicit arcs) from the given \a node.
  bool disconnect(Component* node, bool explicitOnly) override
  {
    bool removedAnArc = false;
    this->evaluateArcs<evaluators::DeleteArcs>(node, explicitOnly, removedAnArc);
    return removedAnArc;
  }

  /**\brief Dump the resource nodes and arcs to a file.
    *
    * This is a convenience method for debugging; its arguments are
    * easy to pass to an interactive debugger. Internally, it calls
    * evaluateArcs<evaluators::Dump>(...) to produce output.  See
    * evaluators::Dump for more information.
    */
  void dump(const std::string& filename, const std::string& mimeType = "text/vnd.graphviz")
    const override
  {
    std::ostream* stream = filename.empty() ? &std::cout : new std::ofstream(filename.c_str());
    if (stream && stream->good())
    {
      evaluators::Dump dump(mimeType);
      this->evaluateArcs<evaluators::Dump>(*stream, dump);
    }
    if (!filename.empty())
    {
      delete stream;
    }
  }

protected:
  std::size_t eraseNodes(const smtk::graph::ComponentPtr& node) override
  {
    return NodeContainer::eraseNodes(node) > 0;
  }

  /// Insert a node without performing any type-safety checks.
  bool insertNode(const smtk::graph::ComponentPtr& node) override
  {
    return NodeContainer::insertNode(node);
  }

  /// Perform a run-time check to validate that a node is acceptable to this resource.
  bool isNodeTypeAcceptable(const smtk::graph::ComponentPtr& node) override
  {
    smtk::string::Token typeToken = node->typeName();
    return m_nodeTypes.find(typeToken) != m_nodeTypes.end();
  }

  /// A functor to create smtk::string::Tokens of accepted node types.
  struct NodeTypeNames
  {
    NodeTypeNames(std::set<smtk::string::Token>& names)
      : m_names(names)
    {
    }

    template<typename T>
    void evaluate(std::size_t ii)
    {
      (void)ii;
      smtk::string::Token name = smtk::common::typeName<T>();
      m_names.insert(name);
    }

    std::set<smtk::string::Token>& m_names;
  };

  void initializeResource()
  {
    // Cache a set of string tokens holding node types.
    NodeTypeNames nodeTypeNames(m_nodeTypes);
    smtk::tuple_evaluate<typename Traits::NodeTypes>(nodeTypeNames);
  }

  Resource(smtk::resource::ManagerPtr manager = nullptr)
    : Superclass(manager)
    , m_arcs(identity<typename detail::GraphTraits<Traits>::ArcTypes>())
  {
    this->initializeResource();
  }

  Resource(const smtk::common::UUID& uid, smtk::resource::ManagerPtr manager = nullptr)
    : Superclass(uid, manager)
    , m_arcs(identity<typename detail::GraphTraits<Traits>::ArcTypes>())
  {
    this->initializeResource();
  }

  smtk::string::Token m_templateType;
  std::size_t m_templateVersion{ 0 };
  ArcMap m_arcs;
  std::set<smtk::string::Token> m_nodeTypes;
  std::set<smtk::string::Token> m_arcTypes;
};

template<typename Traits>
bool Resource<Traits>::setTemplateType(const smtk::string::Token& templateType)
{
  if (m_templateType == templateType)
  {
    return false;
  }
  m_templateType = templateType;
  return true;
}

template<typename Traits>
bool Resource<Traits>::setTemplateVersion(std::size_t templateVersion)
{
  if (m_templateVersion == templateVersion || templateVersion == 0)
  {
    return false;
  }
  m_templateVersion = templateVersion;
  return true;
}

template<typename Traits>
std::shared_ptr<smtk::resource::Resource> Resource<Traits>::clone(
  smtk::resource::CopyOptions& options) const
{
  std::shared_ptr<smtk::graph::ResourceBase> result;
  if (auto rsrcMgr = this->manager())
  {
    result = std::dynamic_pointer_cast<ResourceBase>(rsrcMgr->create(this->typeName()));
  }
  if (result)
  {
    this->prepareClone(options, result);
  }
  return result;
}

template<typename Traits>
void Resource<Traits>::prepareClone(
  smtk::resource::CopyOptions& options,
  const std::shared_ptr<smtk::graph::ResourceBase>& result) const
{
  using smtk::resource::CopyOptions;

  if (!result)
  {
    return;
  }

  // Insert the source of the original into the object mapping
  // so its properties can be copied if need
  options.objectMapping()[this->id()] = result.get();

  if (this->isNameSet())
  {
    result->setName(this->name());
  }

  if (options.copyLocation())
  {
    result->setLocation(this->location());
  }

  result->copyUnitSystem(shared_from_this(), options);

  if (options.copyTemplateData() || options.copyComponents())
  {
    if (options.copyTemplateData())
    {
      result->setTemplateType(this->templateType());
      auto version = this->templateVersion();
      result->setTemplateVersion(version);
    }

    // TODO: Copy any run-time arc types.
  }
}

template<typename Traits>
bool Resource<Traits>::copyInitialize(
  const std::shared_ptr<const smtk::resource::Resource>& source,
  smtk::resource::CopyOptions& options)
{
  const auto& graphSource = std::dynamic_pointer_cast<const Resource<Traits>>(source);
  if (!graphSource)
  {
    smtkErrorMacro(
      options.log(),
      "Resource types do not match (" << source->typeName() << " vs " << this->typeName() << ").");
    return false;
  }

  // Copy nodes
  // Note that this simply constructs new nodes of the same type
  // assuming they can be created with default constructors.
  // If your subclass requires something fancier, you should
  // override copyInitialize().
  if (options.copyComponents())
  {
    smtk::resource::Component::Visitor copyComponent =
      [&](const std::shared_ptr<smtk::resource::Component>& comp) {
        auto node = this->createNodeOfType(comp->typeToken());
        options.objectMapping()[comp->id()] = node;
      };
    source->visit(copyComponent);
  }

  // Copy property data
  this->copyProperties(source, options);

  // Copy renderable geometry
  this->copyGeometry(graphSource, options);

  // Copy arcs
  this->arcs().copyArcs(graphSource.get(), options, this);

  return true;
}

template<typename Traits>
bool Resource<Traits>::copyFinalize(
  const std::shared_ptr<const smtk::resource::Resource>& source,
  smtk::resource::CopyOptions& options)
{
  // Copy links.
  this->copyLinks(source, options);

  // Provide target nodes with an opportunity to copy state data from their source nodes.
  if (options.copyComponents())
  {
    smtk::resource::Component::Visitor assignComponent =
      [&](const std::shared_ptr<smtk::resource::Component>& comp) {
        auto sourceNode = std::dynamic_pointer_cast<smtk::graph::Component>(comp);
        if (sourceNode && !options.shouldOmitId(sourceNode->id()))
        {
          if (
            auto* targetNode =
              options.targetObjectFromSourceId<smtk::graph::Component>(sourceNode->id()))
          {
            targetNode->assign(sourceNode, options);
          }
          else
          {
            smtkErrorMacro(
              options.log(),
              "Source node mapping for " << source->id() << ", \"" << sourceNode->name()
                                         << "\", did not produce a target.");
          }
        }
      };
    source->visit(assignComponent);
  }
  return true;
}

} // namespace graph
} // namespace smtk

#endif // smtk_graph_Resource_h
