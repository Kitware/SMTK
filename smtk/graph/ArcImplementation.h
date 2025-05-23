//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_graph_ArcImplementation_h
#define smtk_graph_ArcImplementation_h

// The order of inclusion for these headers is significant.
// Do NOT reorder them alphabetically, nor allow clang-format to do so.
#include "smtk/common/TypeName.h"
#include "smtk/graph/detail/TypeTraits.h"
// Do not reorder:
#include "smtk/graph/RuntimeArcEndpoint.h"
// RuntimeArcEndpoint.h must come before ArcImplementationBase.h here
// but there are other instances where ArcImplementationBase should not
// be included while RuntimeArcEndpoint should be.
#include "smtk/graph/ArcImplementationBase.h"
#include "smtk/resource/Component.h"
#include "smtk/string/Token.h"

#include "smtk/common/CompilerInformation.h"

// clang-format off
// The default template parameter provided for RuntimeInVisitor causes a
// spurious warning with VS2019 until _MSC_FULL_VER >= 192930040. See here:
// https://developercommunity.visualstudio.com/t/Unexplained-warning-C4348-redefinition-o/945050?q=%5BFixed+in%3A+Visual+Studio+2019+version+16.9+Preview+4%5D
// We disable this warning for older compilers.
#if defined(_MSC_VER_FULL) && _MSC_FULL_VER < 192930040
  __pragma(warning(push))
  __pragma(warning(disable : 4348)) /*redefinition of default parameter*/
#endif
  // clang-format on

  namespace smtk
{
  namespace graph
  {

  template<Directionality>
  struct RuntimeArc;

  // Forward-declare the arc adaptor classes:
  template<typename TraitsType>
  class ArcImplementation;
  template<typename TraitsType, typename Const, typename Outgoing>
  class ArcEndpointInterface;

  /**\brief A wrapper around arc type-traits classes that provides API.
  *
  * Arc traits may not provide implementations for all methods (or indeed, any at
  * all in the case of explicit arcs). This class either forwards calls to the
  * traits object or provides implementations for methods itself.
  */
  template<typename ArcTraits>
  class SMTK_ALWAYS_EXPORT ArcImplementation : public ArcImplementationBase
  {
  public:
    using Traits = ArcTraits; // Allow classes to inspect our input parameter.

    ArcImplementation() = default;
    ArcImplementation(const Traits& traits)
      : m_data(traits)
    {
    }

    /// If "truthy," this arc is considered directed rather than undirected.
    using Directed = typename ArcTraits::Directed;
    /// The type of node that these arcs originate from.
    using FromType = typename ArcTraits::FromType;
    /// The type of node that these arcs point to.
    using ToType = typename ArcTraits::ToType;
    /// If "truthy," this arc will report incoming/outgoing nodes in a (user-specified) order.
    using Ordered = typename ArcProperties<ArcTraits>::isOrdered;
    /**\brief If "truthy," methods that edit arcs may sometimes return true.
    *
    * If "falsey," methods for editing arcs will always return false.
    * This type can be used to optimize at compile time (by omitting
    * code that tests the return value of methods like connect, disconnect,
    * erase, clear, etc.).
    */
    using Mutable = typename ArcProperties<ArcTraits>::isMutable;
    /// If "truthy," arc endpoint connections are explicitly stored by SMTK.
    using Explicit = typename ArcProperties<ArcTraits>::isExplicit;

    /// The minimum out-degree of a FromType node. This is not enforced.
    static constexpr std::size_t MinOutDegree = minOutDegree<ArcTraits>(unconstrained());
    /// The minimum in-degree of a ToType node. This is not enforced.
    static constexpr std::size_t MinInDegree = minInDegree<ArcTraits>(unconstrained());
    /// The maximum out-degree of a FromType node. This is enforced.
    static constexpr std::size_t MaxOutDegree = maxOutDegree<ArcTraits>(unconstrained());
    /// The maximum in-degree of a ToType node. This is enforced.
    static constexpr std::size_t MaxInDegree = maxInDegree<ArcTraits>(unconstrained());
    static_assert(
      Directed::value || !std::is_same<FromType, ToType>::value ||
        (MaxOutDegree == MaxInDegree && MinOutDegree == MinInDegree),
      R"(
    Undirected arcs with identical From/To types must
    have identical in- and out-degree constraints.)");

    /// Return whether arcs of this type are ordered (true) or unordered (false).
    bool isOrdered() const { return Ordered::value; }

    /// Return whether arcs of this type are directed (true) or undirected (false).
    bool isDirected() const { return Directed::value; }

    /// Return whether arcs of this type may be edited.
    bool isMutable() const { return Mutable::value; }

    /// Return whether arcs of this type are explicit.
    bool isExplicit() const { return Explicit::value; }

    /// Return the type of arc this class implements.
    std::string typeName() const override { return smtk::common::typeName<ArcTraits>(); }

    Directionality directionality() const override
    {
      return this->isDirected() ? Directionality::IsDirected : Directionality::IsUndirected;
    }
    bool mutability() const override { return this->isMutable(); }
    bool explicitStorage() const override { return this->isExplicit(); }

  protected:
    template<
      typename U = typename ArcProperties<ArcTraits>::template hasFromNodeSpecs<ArcTraits>::type>
    typename std::enable_if<!U::value, std::unordered_set<smtk::string::Token>>::type _fromTypes()
      const
    {
      return { { ("'" + smtk::common::typeName<typename ArcTraits::FromType>() + "'") } };
    }

    template<
      typename U = typename ArcProperties<ArcTraits>::template hasFromNodeSpecs<ArcTraits>::type>
    typename std::enable_if<U::value, std::unordered_set<smtk::string::Token>>::type _fromTypes()
      const
    {
      return m_data.traits().m_fromNodeSpecs;
    }

    template<
      typename U = typename ArcProperties<ArcTraits>::template hasToNodeSpecs<ArcTraits>::type>
    typename std::enable_if<!U::value, std::unordered_set<smtk::string::Token>>::type _toTypes()
      const
    {
      return { { ("'" + smtk::common::typeName<typename ArcTraits::ToType>() + "'") } };
    }

    template<
      typename U = typename ArcProperties<ArcTraits>::template hasToNodeSpecs<ArcTraits>::type>
    typename std::enable_if<U::value, std::unordered_set<smtk::string::Token>>::type _toTypes()
      const
    {
      return m_data.traits().m_toNodeSpecs;
    }

  public:
    std::unordered_set<smtk::string::Token> fromTypes() const override
    {
      return this->_fromTypes();
    }

    std::unordered_set<smtk::string::Token> toTypes() const override { return this->_toTypes(); }

    /// The minimum out-degree of a FromType node. This is not enforced.
    std::size_t minimumOutDegree() const override { return MinOutDegree; }
    /// The minimum in-degree of a ToType node. This is not enforced.
    std::size_t minimumInDegree() const override { return MinInDegree; }
    /// The maximum out-degree of a FromType node. This is enforced.
    std::size_t maximumOutDegree() const override { return MaxOutDegree; }
    /// The maximum in-degree of a ToType node. This is enforced.
    std::size_t maximumInDegree() const override { return MaxInDegree; }

    /// Return true if there are no arcs of this type in the resource.
    ///
    /// Note that this method may return false even when there are no
    /// arcs of this type because the arc-traits object does not provide
    /// an efficient way to obtain the number of arcs.
    ///@{
    template<
      typename U = typename ArcProperties<Traits>::template hasSize<
        detail::SelectArcContainer<Traits, Traits>>>
    typename std::enable_if<U::value, bool>::type empty() const
    {
      auto result = m_data.size();
      return result == 0;
    }

    template<
      typename U = typename ArcProperties<Traits>::template hasSize<
        detail::SelectArcContainer<Traits, Traits>>>
    typename std::enable_if<!U::value, bool>::type empty() const
    {
      return false;
    }
    ///@}

  protected:
    /// A functor that either calls inVisitor (on arcs with reverse indices) or does nothing.
    template<
      typename TraitsType,
      bool IsBidirectional = !ArcProperties<TraitsType>::isOnlyForwardIndexed::value>
    struct RuntimeInVisitor;

    template<typename TraitsType>
    struct RuntimeInVisitor<TraitsType, true>
    {
      smtk::common::Visited operator()(
        const ArcImplementation<TraitsType>* self,
        const ToType* node,
        std::function<smtk::common::Visit(const Component*)> visitor) const
      {
        return self->inVisitor(node, [&](const FromType* from) { return visitor(from); });
      }
    };

    template<typename TraitsType>
    struct RuntimeInVisitor<TraitsType, false>
    {
      smtk::common::Visited operator()(
        const ArcImplementation<TraitsType>*,
        const ToType*,
        std::function<smtk::common::Visit(const Component*)>) const
      {
        return smtk::common::Visited::Empty;
      }
    };

  public:
    /// Return a "container" of outgoing arcs of the given \a from node.
    ///@{
    // const version
    RuntimeArcEndpoint<ConstArc> outgoingRuntime(const Component* from) const override
    {
      const auto* properType = dynamic_cast<const FromType*>(from);
      if (!properType)
      {
        return RuntimeArcEndpoint<ConstArc>();
      }

      return RuntimeArcEndpoint<ConstArc>(
        this->typeName(),
        from,
        true,
        // Degree-limits functor
        [this](bool isMax) { return isMax ? maximumOutDegree() : minimumOutDegree(); },
        // Degree (node valence) functor
        [this, properType]() { return this->outDegree(properType); },
        // Graph contains arc functor
        [this, properType](const Component* to) {
          if (const auto* properTo = dynamic_cast<const ToType*>(to))
          {
            return this->contains(properType, properTo);
          }
          return false;
        },
        // Arc insertion functor
        [this, properType](const Component*, const Component*, const Component*) { return false; },
        // Arc removal functor
        [this, properType](const Component*) { return false; },
        // Arc visitor functor
        [this, properType](std::function<smtk::common::Visit(const Component*)> visitor) {
          return this->outVisitor(properType, [&](const ToType* node) { return visitor(node); });
        });
    }
    // non-const version
    RuntimeArcEndpoint<NonConstArc> outgoingRuntime(const Component* from) override
    {
      const auto* properType = dynamic_cast<const FromType*>(from);
      if (!properType)
      {
        return RuntimeArcEndpoint<NonConstArc>();
      }

      return RuntimeArcEndpoint<NonConstArc>(
        this->typeName(),
        from,
        true,
        // Degree-limits functor
        [this](bool isMax) { return isMax ? maximumOutDegree() : minimumOutDegree(); },
        // Degree (node valence) functor
        [this, properType]() { return this->outDegree(properType); },
        // Graph contains arc functor
        [this, properType](const Component* to) {
          if (const auto* properTo = dynamic_cast<const ToType*>(to))
          {
            return this->contains(properType, properTo);
          }
          return false;
        },
        // Arc insertion functor
        [this,
         properType](const Component* to, const Component* beforeTo, const Component* beforeFrom) {
          if (
            (beforeTo && !dynamic_cast<const ToType*>(beforeTo)) ||
            (beforeFrom && !dynamic_cast<const FromType*>(beforeFrom)))
          {
            // TODO: Emit warning?
            return false;
          }
          if (const auto* properTo = dynamic_cast<const ToType*>(to))
          {
            auto* self = const_cast<ArcImplementation<ArcTraits>*>(this);
            return self->connect(
              properType,
              properTo,
              dynamic_cast<const FromType*>(beforeFrom),
              dynamic_cast<const ToType*>(beforeTo));
          }
          return false;
        },
        // Arc removal functor
        [this, properType](const Component* to) {
          if (const auto* properTo = dynamic_cast<const ToType*>(to))
          {
            auto* self = const_cast<ArcImplementation<ArcTraits>*>(this);
            return self->disconnect(properType, properTo);
          }
          return false;
        },
        // Arc visitor functor
        [this, properType](std::function<smtk::common::Visit(const Component*)> visitor) {
          return this->outVisitor(properType, [&](const ToType* node) { return visitor(node); });
        });
    }
    ///@}

    /// Return a "container" of incoming arcs of the given \a to node.
    ///@{
    // const version
    RuntimeArcEndpoint<ConstArc> incomingRuntime(const Component* to) const override
    {
      const auto* properType = dynamic_cast<const ToType*>(to);
      if (!properType)
      {
        return RuntimeArcEndpoint<ConstArc>();
      }

      return RuntimeArcEndpoint<ConstArc>(
        this->typeName(),
        to,
        false,
        // Degree-limits functor
        [this](bool isMax) { return isMax ? maximumInDegree() : minimumInDegree(); },
        // Degree (node valence) functor
        [this, properType]() {
          std::size_t result = 0;
          if (!properType)
          {
            return result;
          }
          RuntimeInVisitor<ArcTraits>()(this, properType, [&result](const Component*) {
            ++result;
            return smtk::common::Visit::Continue;
          });
          return result;
        },
        // Graph contains arc functor
        [this, properType](const Component* from) {
          if (const auto* properFrom = dynamic_cast<const FromType*>(from))
          {
            return this->contains(properFrom, properType);
          }
          return false;
        },
        // Arc insertion functor
        [this, properType](const Component*, const Component*, const Component*) { return false; },
        // Arc removal functor
        [this, properType](const Component*) { return false; },
        // Arc visitor functor
        [this, properType](std::function<smtk::common::Visit(const Component*)> visitor) {
          return RuntimeInVisitor<ArcTraits>()(this, properType, visitor);
        });
    }
    // non-const version
    RuntimeArcEndpoint<NonConstArc> incomingRuntime(const Component* to) override
    {
      const auto* properType = dynamic_cast<const ToType*>(to);
      if (!properType)
      {
        return RuntimeArcEndpoint<NonConstArc>();
      }

      return RuntimeArcEndpoint<NonConstArc>(
        this->typeName(),
        to,
        false,
        // Degree-limits functor
        [this](bool isMax) { return isMax ? maximumInDegree() : minimumInDegree(); },
        // Degree (node valence) functor
        [this, properType]() {
          std::size_t result = 0;
          if (!properType)
          {
            return result;
          }
          RuntimeInVisitor<ArcTraits>()(this, properType, [&result](const Component*) {
            ++result;
            return smtk::common::Visit::Continue;
          });
          return result;
        },
        // Graph contains arc functor
        [this, properType](const Component* from) {
          if (const auto* properFrom = dynamic_cast<const FromType*>(from))
          {
            return this->contains(properFrom, properType);
          }
          return false;
        },
        // Arc insertion functor
        [this, properType](
          const Component* from, const Component* beforeFrom, const Component* beforeTo) {
          if (
            (beforeTo && !dynamic_cast<const ToType*>(beforeTo)) ||
            (beforeFrom && !dynamic_cast<const FromType*>(beforeFrom)))
          {
            // TODO: Emit warning?
            return false;
          }
          if (const auto* properFrom = dynamic_cast<const FromType*>(from))
          {
            auto* self = const_cast<ArcImplementation<ArcTraits>*>(this);
            return self->connect(
              properFrom,
              properType,
              dynamic_cast<const FromType*>(beforeFrom),
              dynamic_cast<const ToType*>(beforeTo));
          }
          return false;
        },
        // Arc removal functor
        [this, properType](const Component* from) {
          if (auto* properFrom = dynamic_cast<const FromType*>(from))
          {
            auto* self = const_cast<ArcImplementation<ArcTraits>*>(this);
            return self->disconnect(properFrom, properType);
          }
          return false;
        },
        // Arc visitor functor
        [this, properType](std::function<smtk::common::Visit(const Component*)> visitor) {
          return RuntimeInVisitor<ArcTraits>()(this, properType, visitor);
        });
    }
    ///@}

    /**\brief Test whether an arc from \a from to \a to exists.
    *
    */
    ///@{
    template<
      typename U = typename ArcProperties<Traits>::template hasContains<
        detail::SelectArcContainer<Traits, Traits>>>
    typename std::enable_if<!U::value, bool>::type contains(const FromType* from, const ToType* to)
      const
    {
      // Provide a (slow) implementation for arc types
      // that do not provide their own (fast) implementation.
      bool result = false;
      if (!from)
      {
        return result;
      }
      this->outVisitor(from, [&result, &to](const ToType* node) {
        if (node == to)
        {
          result = true;
          return smtk::common::Visit::Halt;
        }
        return smtk::common::Visit::Continue;
      });
      return result;
    };

    template<
      typename U = typename ArcProperties<Traits>::template hasContains<
        detail::SelectArcContainer<Traits, Traits>>>
    typename std::enable_if<U::value, bool>::type contains(const FromType* from, const ToType* to)
      const
    {
      bool result = m_data.contains(from, to);
      return result;
    };
    ///@}

    /**\brief Visit all nodes which have outgoing arcs of this type.
    *
    * If the traits object does not provide its own implementation,
    * the ArcImplementation will simply visit all nodes of type
    * FromType and invoke the visitor on each.
    *
    * The intent is for traits objects to provide a fast method that
    * only visits nodes which have outgoing arcs of this type.
    */
    ///@{
    template<
      typename U = typename ArcProperties<Traits>::template hasOutNodeVisitor<
        detail::SelectArcContainer<Traits, Traits>>,
      typename ResourcePtr,
      typename Functor>
    typename std::enable_if<!U::value, smtk::common::Visited>::type visitAllOutgoingNodes(
      ResourcePtr rsrc,
      Functor ff) const
    {
      // Provide a (slow) implementation for arc types
      // that do not provide their own (fast) implementation.
      smtk::common::Visited result = smtk::common::Visited::Empty;
      if (!rsrc)
      {
        return result;
      }
      smtk::common::VisitorFunctor<Functor> nodeVisitor(ff);
      // FIXME: The default resource visitor does not allow early termination.
      std::function<void(const std::shared_ptr<smtk::resource::Component>&)> compVisitor =
        [&](const smtk::resource::ComponentPtr& component) {
          if (const auto* node = dynamic_cast<const typename Traits::FromType*>(component.get()))
          {
            nodeVisitor(node);
          }
        };
      rsrc->visit(compVisitor);
      result = smtk::common::Visited::All;
      return result;
    };

    template<
      typename U = typename ArcProperties<Traits>::template hasOutNodeVisitor<
        detail::SelectArcContainer<Traits, Traits>>,
      typename ResourcePtr,
      typename Functor>
    typename std::enable_if<U::value, smtk::common::Visited>::type visitAllOutgoingNodes(
      ResourcePtr rsrc,
      Functor ff) const
    {
      smtk::common::Visited result = m_data.visitAllOutgoingNodes(rsrc, ff);
      return result;
    };
    ///@}

    /// Visit all nodes with outgoing run-time arcs.
    smtk::common::Visited visitOutgoingNodes(
      const smtk::graph::ResourceBase* resource,
      smtk::string::Token arcTypeName,
      RuntimeNodeVisitor visitor) const override
    {
      (void)arcTypeName;
      return this->visitAllOutgoingNodes(
        const_cast<ResourceBase*>(resource),
        [&](const typename Traits::FromType* node) { return visitor(node); });
    }

    /**\brief Visit all nodes which have incoming arcs of this type.
    *
    * If the traits object does not provide its own implementation,
    * the ArcImplementation will simply visit all nodes of type
    * ToType and invoke the visitor on each.
    *
    * The intent is for traits objects to provide a fast method that
    * only visits nodes which have incoming arcs of this type.
    */
    ///@{
    template<
      typename U = typename ArcProperties<Traits>::template hasInNodeVisitor<
        detail::SelectArcContainer<Traits, Traits>>,
      typename ResourcePtr,
      typename Functor>
    typename std::enable_if<!U::value, smtk::common::Visited>::type visitAllIncomingNodes(
      ResourcePtr rsrc,
      Functor ff) const
    {
      // Provide a (slow) implementation for arc types
      // that do not provide their own (fast) implementation.
      smtk::common::Visited result = smtk::common::Visited::Empty;
      if (!rsrc)
      {
        return result;
      }
      smtk::common::VisitorFunctor<Functor> nodeVisitor(ff);
      // FIXME: The default resource visitor does not allow early termination.
      std::function<void(const std::shared_ptr<smtk::resource::Component>&)> compVisitor =
        [&](const smtk::resource::ComponentPtr& component) {
          if (const auto* node = dynamic_cast<const typename Traits::ToType*>(component.get()))
          {
            nodeVisitor(node);
          }
        };
      rsrc->visit(compVisitor);
      result = smtk::common::Visited::All;
      return result;
    };

    template<
      typename U = typename ArcProperties<Traits>::template hasInNodeVisitor<
        detail::SelectArcContainer<Traits, Traits>>,
      typename ResourcePtr,
      typename Functor>
    typename std::enable_if<U::value, smtk::common::Visited>::type visitAllIncomingNodes(
      ResourcePtr rsrc,
      Functor ff) const
    {
      smtk::common::Visited result = m_data.visitAllIncomingNodes(rsrc, ff);
      return result;
    };
    ///@}

    /**\brief Determine whether the given arc is allowed.
    *
    * If the decorated traits object provides an "accepts()"
    * method, return the result of that. Otherwise, consider
    * only whether the compile-time constraints are satisfied.
    *
    * Note that if "accepts()" is not provided, this method
    * may eventually be implemented by checking the maximum
    * incoming and outgoing degree constraints; that can be
    * slow since degree() could be called on each node.
    */
    ///@{
    template<
      typename U = typename ArcProperties<Traits>::template hasAccepts<
        detail::SelectArcContainer<Traits, Traits>>>
    typename std::enable_if<U::value, bool>::type accepts(
      const FromType* from,
      const ToType* to,
      const FromType* beforeFrom = nullptr,
      const ToType* beforeTo = nullptr) const
    {
      return m_data.accepts(from, to, beforeFrom, beforeTo);
    }

    template<
      typename U = typename ArcProperties<Traits>::template hasAccepts<
        detail::SelectArcContainer<Traits, Traits>>>
    typename std::enable_if<!U::value, bool>::type accepts(
      const FromType* from,
      const ToType* to,
      const FromType* beforeFrom = nullptr,
      const ToType* beforeTo = nullptr) const
    {
      (void)beforeFrom;
      (void)beforeTo;
      if (!from || !to)
      {
        return false;
      }
      if (MaxOutDegree != unconstrained() && this->outDegree(from) + 1 >= MaxOutDegree)
      {
        return false;
      }
#if 0
    // TODO: Check inDegree constraint. This is
    // complicated by the fact that inDegree may not be
    // computable if the arc is forward-only-indexed.
    if (MaxInDegree != unconstrained() &&
      this->inDegree(to) + 1 >= MaxInDegree)
    {
      return false;
    }
#endif
      if (this->contains(from, to))
      {
        return false;
      }
      return true;
    }
    ///@}

    bool acceptsRuntime(Component* from, Component* to, Component* beforeFrom, Component* beforeTo)
      const override
    {
      bool ok = this->accepts(
        dynamic_cast<const FromType*>(from),
        dynamic_cast<const ToType*>(to),
        dynamic_cast<const FromType*>(beforeFrom),
        dynamic_cast<const ToType*>(beforeTo));
      return ok;
    };

    /**\brief Insert an arc from \a from to \a to,
    *       optionally ordered by \a beforeFrom and \a beforeTo.
    *
    * If the arc is ordered, then \a beforeFrom indicates where
    * \a from should be placed in the order of incoming nodes and
    * similarly for \a beforeTo and \a to.
    * If the arc is not ordered, then \a beforeFrom and \a beforeTo
    * are ignored.
    */
    ///@{
    template<typename U = Mutable>
    typename std::enable_if<!U::value, bool>::type connect(
      const FromType* from,
      const ToType* to,
      const FromType* beforeFrom = nullptr,
      const ToType* beforeTo = nullptr)
    {
      (void)from;
      (void)to;
      (void)beforeFrom;
      (void)beforeTo;
      return false;
    }

    template<typename U = Mutable>
    typename std::enable_if<U::value, bool>::type connect(
      const FromType* from,
      const ToType* to,
      const FromType* beforeFrom = nullptr,
      const ToType* beforeTo = nullptr)
    {
      bool result = m_data.connect(from, to, beforeFrom, beforeTo);
      return result;
    }
    ///@}

    /**\brief Remove an arc from \a from to \a to.
    *
    */
    ///@{
    template<typename U = Mutable>
    typename std::enable_if<!U::value, bool>::type disconnect(
      const FromType* from,
      const ToType* to)
    {
      (void)from;
      (void)to;
      return false;
    }

    template<typename U = Mutable>
    typename std::enable_if<U::value, bool>::type disconnect(const FromType* from, const ToType* to)
    {
      bool result = m_data.disconnect(from, to);
      return result;
    }
    ///@}

    /// Visit nodes attached via outgoing arcs.
    template<typename Functor>
    smtk::common::Visited outVisitor(const FromType* from, Functor visitor) const
    {
      return m_data.outVisitor(from, visitor);
    }

    /**\brief Compute the out-degree of the node.
    */
    ///@{
    template<
      typename U = typename ArcProperties<Traits>::template hasOutDegree<
        detail::SelectArcContainer<Traits, Traits>>,
      typename V =
        typename ArcProperties<detail::SelectArcContainer<Traits, Traits>>::isAutoUndirected>
    typename std::enable_if<!U::value && !V::value, std::size_t>::type outDegree(
      const FromType* from) const
    {
      // Provide a (slow) implementation for arc types
      // that do not provide their own (fast) implementation (in
      // the case where arcs are **not** auto-undirected).
      std::size_t result = 0;
      if (!from)
      {
        return result;
      }
      this->outVisitor(from, [&result](const ToType*) { ++result; });
      return result;
    };

    template<
      typename U = typename ArcProperties<Traits>::template hasOutDegree<
        detail::SelectArcContainer<Traits, Traits>>,
      typename V =
        typename ArcProperties<detail::SelectArcContainer<Traits, Traits>>::isAutoUndirected>
    typename std::enable_if<!U::value && V::value, std::size_t>::type outDegree(
      const FromType* from) const
    {
      // Provide a (slow) implementation for arc types
      // that do not provide their own (fast) implementation (in
      // the case where arcs **are** auto-undirected).
      std::size_t result = 0;
      if (!from)
      {
        return result;
      }
      this->outVisitor(from, [&result](const ToType*) { ++result; });
      // Auto-undirected arcs may have to/from nodes flipped... look
      // for \a from in the reverse map. We are guaranteed to have
      // an in-visitor for undirected arcs.
      this->inVisitor(
        reinterpret_cast<const ToType*>(from), [&result](const FromType*) { ++result; });

      return result;
    };

    template<
      typename U = typename ArcProperties<Traits>::template hasOutDegree<
        detail::SelectArcContainer<Traits, Traits>>>
    typename std::enable_if<U::value, std::size_t>::type outDegree(const FromType* from) const
    {
      std::size_t result = m_data.outDegree(from);
      return result;
    };
    ///@}

    /**\brief Compute the in-degree of the node.
    */
    ///@{
    template<
      typename U = typename ArcProperties<Traits>::template hasInDegree<
        detail::SelectArcContainer<Traits, Traits>>,
      typename V =
        typename ArcProperties<detail::SelectArcContainer<Traits, Traits>>::isAutoUndirected>
    typename std::enable_if<!U::value && !V::value, std::size_t>::type inDegree(
      const ToType* to) const
    {
      // Provide a (slow) implementation for arc types
      // that do not provide their own (fast) implementation (in
      // the case where arcs are **not** auto-undirected).
      std::size_t result = 0;
      if (!to)
      {
        return result;
      }
      this->inVisitor(to, [&result](const FromType*) { ++result; });
      return result;
    };

    template<
      typename U = typename ArcProperties<Traits>::template hasInDegree<
        detail::SelectArcContainer<Traits, Traits>>,
      typename V =
        typename ArcProperties<detail::SelectArcContainer<Traits, Traits>>::isAutoUndirected>
    typename std::enable_if<!U::value && V::value, std::size_t>::type inDegree(
      const ToType* to) const
    {
      // Provide a (slow) implementation for arc types
      // that do not provide their own (fast) implementation (in
      // the case where arcs **are** auto-undirected).
      std::size_t result = 0;
      if (!to)
      {
        return result;
      }
      this->inVisitor(to, [&result](const ToType*) { ++result; });
      // Auto-undirected arcs may have to/from nodes flipped... look
      // for \a to in the reverse map. We are guaranteed to have
      // an in-visitor for undirected arcs.
      this->inVisitor(to, [&result](const ToType*) { ++result; });

      return result;
    };

    template<
      typename U = typename ArcProperties<Traits>::template hasInDegree<
        detail::SelectArcContainer<Traits, Traits>>>
    typename std::enable_if<U::value, std::size_t>::type inDegree(const ToType* to) const
    {
      std::size_t result = m_data.inDegree(to);
      return result;
    };
    ///@}

  protected:
    // Only implement inVisitor for ArcTraits that are bidirectional.
    template<
      typename Functor,
      bool IsBidirectional = !ArcProperties<ArcTraits>::isOnlyForwardIndexed::value>
    smtk::common::Visited inVisitorDetail(const ToType* to, Functor visitor) const
    {
      return m_data.inVisitor(to, visitor);
    }

  public:
    /**\brief Visit nodes attached via incoming arcs.
    *
    * This method will only be provided for
    * + explicit arc-traits which do **not** include a truthy ForwardIndexOnly type-alias; and
    * + implicit arc-traits which provide an inVisitor method.
    */
    template<typename Functor>
    smtk::common::Visited inVisitor(const ToType* to, Functor visitor) const
    {
      return this->inVisitorDetail<Functor, true>(to, visitor);
    }

    /// Return a "container" of outgoing arcs of the given \a from node.
    ArcEndpointInterface<ArcTraits, ConstArc, OutgoingArc> outgoing(const FromType* from) const
    {
      return ArcEndpointInterface<ArcTraits, ConstArc, OutgoingArc>(this, from);
    }
    ArcEndpointInterface<ArcTraits, NonConstArc, OutgoingArc> outgoing(FromType* from)
    {
      return ArcEndpointInterface<ArcTraits, NonConstArc, OutgoingArc>(this, from);
    }

    /// Return a "container" of incoming arcs of the given \a to node.
    ArcEndpointInterface<ArcTraits, ConstArc, IncomingArc> incoming(const ToType* to) const
    {
      return ArcEndpointInterface<ArcTraits, ConstArc, IncomingArc>(this, to);
    }
    ArcEndpointInterface<ArcTraits, NonConstArc, IncomingArc> incoming(ToType* to)
    {
      return ArcEndpointInterface<ArcTraits, NonConstArc, IncomingArc>(this, to);
    }

  protected:
    /**\brief Store arc endpoint data.
    *
    * This will be either ArcTraits or ExplicitArcs<ArcTraits>, depending
    * on whether ArcTraits is implicit or explicit.
    * If ArcTraits is implicit, m_data will typically not store
    * components or their IDs directly.
    * If ArcTraits is explicit, m_data will hold a multi-index container
    * of arc endpoints.
    */
    detail::SelectArcContainer<ArcTraits, ArcTraits> m_data;
  };

  /**\brief An object that a node instance can present to access/edit its outgoing/incoming arcs.
  *
  * If \a Outgoing is true, then instances of this class should be constructed with
  * a "from" node's pointer and will act like a container of "to" nodes for the arc type.
  *
  * If \a Outgoing is false, then instances of this class should be constructed with
  * a "to" node's pointer and will act like a container of "from" nodes for the arc type,
  * assuming the arc type provides reverse indexing.
  */
  template<typename TraitsType, typename Const, typename Outgoing>
  class SMTK_ALWAYS_EXPORT ArcEndpointInterface
  {
  public:
    using Traits = TraitsType;
    using FromType = typename TraitsType::FromType;
    using ToType = typename TraitsType::ToType;
    using Directed = typename TraitsType::Directed;
    using Mutable = typename ArcProperties<TraitsType>::isMutable;
    using Ordered = typename ArcProperties<TraitsType>::isOrdered;
    using ImplType = ArcImplementation<TraitsType>;
    using ThisType = typename std::
      conditional<Outgoing::value, typename ImplType::FromType, typename ImplType::ToType>::type;
    using OtherType = typename std::
      conditional<Outgoing::value, typename ImplType::ToType, typename ImplType::FromType>::type;
    using Constness = Const;
    using SelfEndpoint = ArcEndpointInterface<TraitsType, Const, Outgoing>;

    // Prevent this template from being realized when the arc Traits are only forward-indexed.
    static_assert(Outgoing::value || !ArcProperties<Traits>::isOnlyForwardIndexed::value, R"(
        This arc type is not indexed for reverse lookups.
  )");

    /// This is a dummy constructor that is not used by valid code.
    /// Methods that must return a container return instances created
    /// with this constructor, but only after throwing an exception.
    ArcEndpointInterface()
      : m_arcs(nullptr)
      , m_endpoint(nullptr)
    {
    }

    /// Provide a default copy constructor.
    ArcEndpointInterface(const ArcEndpointInterface<TraitsType, Const, Outgoing>&) = default;

    /// Return this endpoint's endpoint.
    typename std::conditional<Const::value, const ThisType*, ThisType*>::type self()
    {
      return m_endpoint;
    }

    // Return constraints on the degree of this endpoint's arcs:
    constexpr std::size_t maxDegree() const
    {
      return Outgoing::value ? maxOutDegree<Traits>(unconstrained())
                             : maxInDegree<Traits>(unconstrained());
    }
    constexpr std::size_t minDegree() const
    {
      return Outgoing::value ? minOutDegree<Traits>(unconstrained())
                             : minInDegree<Traits>(unconstrained());
    }

  protected:
    template<bool Outgoingness, typename Functor>
    struct VisitEndpoints
    {
      smtk::common::Visited operator()(const SelfEndpoint* self, Functor visitor) const
      {
        return self->m_arcs->outVisitor(self->m_endpoint, visitor);
      }
    };

    template<typename Functor>
    struct VisitEndpoints<false, Functor>
    {
      smtk::common::Visited operator()(const SelfEndpoint* self, Functor visitor) const
      {
        return self->m_arcs->inVisitor(self->m_endpoint, visitor);
      }
    };

    template<bool Outgoingness, int Dummy = 0>
    struct ContainsEndpoint
    {
      bool operator()(const SelfEndpoint* self, const ToType* endpoint) const
      {
        return self->m_arcs->contains(self->m_endpoint, endpoint);
      }
    };

    template<int Dummy>
    struct ContainsEndpoint<false, Dummy>
    {
      bool operator()(const SelfEndpoint* self, const FromType* endpoint) const
      {
        return self->m_arcs->contains(endpoint, self->m_endpoint);
      }
    };

    // Provide a functor for computing degree.
    // The default implementation is for outgoing arcs (i.e., outDegree).
    template<bool Outgoingness, int Dummy = 0>
    struct Degree
    {
      std::size_t operator()(const SelfEndpoint* self) const
      {
        return self->m_arcs->outDegree(self->m_endpoint);
      }
    };

    // Specialize the above for incoming degree (inDegree).
    template<int Dummy>
    struct Degree<false, Dummy>
    {
      std::size_t operator()(const SelfEndpoint* self) const
      {
        return self->m_arcs->inDegree(self->m_endpoint);
      }
    };

  public:
    /// Visit nodes attached to the endpoint by arcs of this type.
    template<typename Functor>
    smtk::common::Visited visit(Functor visitor) const
    {
      return VisitEndpoints<Outgoing::value, Functor>()(this, visitor);
    }

    /// Return true if this endpoint is connected to \a node.
    bool contains(const OtherType* node) const
    {
      return ContainsEndpoint<Outgoing::value>()(this, node);
    }

    /**\brief Return the number of arcs of this type that terminate at this endpoint.
    *
    * The variant named size() exists so the endpoint behaves like an STL container.
    */
    ///@{
    std::size_t degree() const
    {
      std::size_t dd = Degree<Outgoing::value>()(this);
      return dd;
    }
    std::size_t size() const { return this->degree(); }
    ///@}

    /// Return whether this endpoint has zero arcs.
    bool empty() const { return this->size() == 0; }

    /**\brief Return the first destination endpoint.
    *
    * This is a convenience method, generally intended
    * for the case when maxDegree() == 1.
    *
    * If no arcs exist, this will return a null pointer.
    *
    * This *may* eventually return a reference to a pointer
    * to allow users to replace the destination node with another,
    * but currently it returns only a value.
    */
    const OtherType* node() const
    {
      const OtherType* result = nullptr;
      this->visit([&result](const OtherType* node) {
        result = node;
        return result ? smtk::common::Visit::Halt : smtk::common::Visit::Continue;
      });
      return result;
    }

    /// STL-container synonym for node():
    const OtherType* front() const { return this->node(); }

    /// Edit methods (only enabled for non-const interfaces).
    ///@{

  protected:
    template<bool Mutability, bool Outgoingness, int Dummy = 0>
    struct Connector
    {
      bool operator()(
        SelfEndpoint* self,
        const OtherType* other,
        const OtherType* beforeOther = nullptr,
        const ThisType* beforeThis = nullptr) const
      {
        return self->m_arcs->connect(self->m_endpoint, other, beforeThis, beforeOther);
      }
    };

    template<int Dummy>
    struct Connector<true, false, Dummy>
    {
      bool operator()(
        SelfEndpoint* self,
        const OtherType* other,
        const OtherType* beforeOther = nullptr,
        const ThisType* beforeThis = nullptr) const
      {
        return self->m_arcs->connect(other, self->m_endpoint, beforeOther, beforeThis);
      }
    };

    // Provide an implementation for immutable arcs that rejects all edits.
    template<bool Outgoingness, int Dummy>
    struct Connector<false, Outgoingness, Dummy>
    {
      bool operator()(
        SelfEndpoint* self,
        const OtherType* other,
        const OtherType* beforeOther = nullptr,
        const ThisType* beforeThis = nullptr) const
      {
        (void)self;
        (void)other;
        (void)beforeOther;
        (void)beforeThis;
        return false;
      }
    };

    template<bool Mutability, bool Outgoingness, int Dummy = 0>
    struct Disconnector
    {
      bool operator()(SelfEndpoint* self, const OtherType* other) const
      {
        bool didDisconnect = self->m_arcs->disconnect(self->m_endpoint, other);
        return didDisconnect;
      }
    };

    template<int Dummy>
    struct Disconnector<true, false, Dummy>
    {
      bool operator()(SelfEndpoint* self, const OtherType* other) const
      {
        bool didDisconnect = self->m_arcs->disconnect(other, self->m_endpoint);
        return didDisconnect;
      }
    };

    template<bool Outgoingness, int Dummy>
    struct Disconnector<false, Outgoingness, Dummy>
    {
      bool operator()(SelfEndpoint* self, const OtherType* other) const
      {
        (void)self;
        (void)other;
        return false;
      }
    };

  public:
    /// Connect the \a other node to this node.
    bool connect(const OtherType* other)
    {
      static_assert(!Constness::value, "A const-referenced node may not be edited.");
      return Connector<Mutable::value, Outgoing::value>()(this, other);
    }

    /// A convenience version of connect() that accepts shared pointers.
    bool connect(const std::shared_ptr<OtherType>& other) { return this->connect(other.get()); }

    /// Insert a new arc connecting this endpoint to the \a other.
    bool insert(
      const OtherType* other,
      const OtherType* beforeOther = nullptr,
      const ThisType* beforeThis = nullptr)
    {
      static_assert(!Constness::value, "A const-referenced node may not be edited.");
      return Connector<Mutable::value, Outgoing::value>()(this, other, beforeThis, beforeOther);
    }

    /// Disconnect the \a other node from this one (i.e., erase an arc).
    bool disconnect(const OtherType* other)
    {
      static_assert(!Constness::value, "A const-referenced node may not be edited.");
      return Disconnector<Mutable::value, Outgoing::value>()(this, other);
    }
    bool erase(const OtherType* other)
    {
      static_assert(!Constness::value, "A const-referenced node may not be edited.");
      return Disconnector<Mutable::value, Outgoing::value>()(this, other);
    }
    /// A convenience version of disconnect() that accepts shared pointers.
    bool disconnect(const std::shared_ptr<OtherType>& other)
    {
      return this->disconnect(other.get());
    }
    /// A convenience version of erase() that accepts shared pointers.
    bool erase(const std::shared_ptr<OtherType>& other) { return this->erase(other.get()); }

    ///@}

    /// Use connect and disconnect to edit all attached arcs until they match \a destNodes.
    /// Return true if modified and false otherwise.
    // template<typename Container>
    // typename std::enable_if<!Const::value, bool>::type
    // operator = (const Container& destNodes);

  protected:
    friend class ArcImplementation<TraitsType>;

    ArcEndpointInterface(
      const ArcImplementation<TraitsType>* arcs,
      typename std::conditional<Const::value, const ThisType*, ThisType*>::type endpoint)
      : m_arcs(const_cast<ArcImplementation<TraitsType>*>(arcs))
      , m_endpoint(endpoint)
    {
    }

    ArcImplementation<TraitsType>* m_arcs;
    typename std::conditional<Const::value, const ThisType*, ThisType*>::type m_endpoint;
  };

  } // namespace graph
} // namespace smtk

#if defined(_MSC_VER_FULL) && _MSC_FULL_VER < 192930040
__pragma(warning(pop))
#endif

#endif // smtk_graph_ArcImplementation_h
