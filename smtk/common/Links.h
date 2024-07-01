//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_common_Links_h
#define smtk_common_Links_h

#include "smtk/common/CompilerInformation.h"

SMTK_THIRDPARTY_PRE_INCLUDE
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/global_fun.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index_container.hpp>
SMTK_THIRDPARTY_POST_INCLUDE

#include <cassert>
#include <functional>
#include <limits>
#include <set>
#include <string>
#include <type_traits>
#include <unordered_set>
#include <utility>

namespace smtk
{
namespace common
{
template<
  typename id_type,
  typename left_type,
  typename right_type,
  typename role_type,
  typename base_type>
class Links;

/// A Link is an object that associates (or "links") two pieces of information
/// together. Since multiple links can exist between the same objects, links
/// have an id for unique identification. Links also have a field to describe
/// their role.
template<
  typename id_type,
  typename left_type,
  typename right_type,
  typename role_type,
  typename base_type>
struct Link : base_type
{
  Link(
    const base_type& base_,
    const id_type& id_,
    const left_type& left_,
    const right_type& right_,
    const role_type& role_)
    : base_type(base_)
    , id(id_)
    , left(left_)
    , right(right_)
    , role(role_)
  {
  }

  Link(
    base_type&& base_,
    const id_type& id_,
    const left_type& left_,
    const right_type& right_,
    const role_type& role_)
    : base_type(base_)
    , id(id_)
    , left(left_)
    , right(right_)
    , role(role_)
  {
  }

  ~Link() override = default;

  id_type id;
  left_type left;
  right_type right;
  role_type role;
};

namespace detail
{
struct NullLinkBase
{
};

/// Tags for access.
struct Id
{
};
struct Left
{
};
struct Right
{
};
struct Role
{
};

using namespace boost::multi_index;

/// A multi-index container that has unique id indexing and non-unique left,
/// right and role indexing. A link_type is also expected; users are optionally
/// able to use template classes that inherit from Link and augment its storage
/// and utility.
template<
  typename id_type,
  typename left_type,
  typename right_type,
  typename role_type,
  typename base_type>
using LinkContainer = boost::multi_index_container<
  Link<id_type, left_type, right_type, role_type, base_type>,
  indexed_by<
    ordered_unique<
      tag<Id>,
      member<
        Link<id_type, left_type, right_type, role_type, base_type>,
        id_type,
        &Link<id_type, left_type, right_type, role_type, base_type>::id>>,
    ordered_non_unique<
      tag<Role>,
      member<
        Link<id_type, left_type, right_type, role_type, base_type>,
        role_type,
        &Link<id_type, left_type, right_type, role_type, base_type>::role>>,
    ordered_non_unique<
      tag<Left>,
      composite_key<
        Link<id_type, left_type, right_type, role_type, base_type>,
        member<
          Link<id_type, left_type, right_type, role_type, base_type>,
          left_type,
          &Link<id_type, left_type, right_type, role_type, base_type>::left>,
        member<
          Link<id_type, left_type, right_type, role_type, base_type>,
          role_type,
          &Link<id_type, left_type, right_type, role_type, base_type>::role>>>,
    ordered_non_unique<
      tag<Right>,
      composite_key<
        Link<id_type, left_type, right_type, role_type, base_type>,
        member<
          Link<id_type, left_type, right_type, role_type, base_type>,
          right_type,
          &Link<id_type, left_type, right_type, role_type, base_type>::right>,
        member<
          Link<id_type, left_type, right_type, role_type, base_type>,
          role_type,
          &Link<id_type, left_type, right_type, role_type, base_type>::role>>>>>;

/// Traits classes for Links. We key off of the tags to return sane responses
/// in the Links class.
template<
  typename id_type,
  typename left_type,
  typename right_type,
  typename role_type,
  typename base_type,
  typename tag>
struct LinkTraits;

template<
  typename id_type,
  typename left_type,
  typename right_type,
  typename role_type,
  typename base_type>
struct LinkTraits<id_type, left_type, right_type, role_type, base_type, Left>
{
  typedef Link<id_type, left_type, right_type, role_type, base_type> Link_;
  typedef Right OtherTag;
  typedef left_type type;
  typedef right_type other_type;
  static const type& value(const Link_& a) { return a.left; }
  static void setValue(Link_& a, const type& v) { a.left = v; }
};

template<
  typename id_type,
  typename left_type,
  typename right_type,
  typename role_type,
  typename base_type>
struct LinkTraits<id_type, left_type, right_type, role_type, base_type, Right>
{
  typedef Link<id_type, left_type, right_type, role_type, base_type> Link_;
  typedef Left OtherTag;
  typedef right_type type;
  typedef left_type other_type;
  static const type& value(const Link_& a) { return a.right; }
  static void setValue(Link_& a, const type& v) { a.right = v; }
};

template<
  typename id_type,
  typename left_type,
  typename right_type,
  typename role_type,
  typename base_type>
struct LinkTraits<id_type, left_type, right_type, role_type, base_type, Role>
{
  typedef Link<id_type, left_type, right_type, role_type, base_type> Link_;
  typedef role_type type;
  static const type& value(const Link_& a) { return a.role; }
  static void setValue(Link_& a, const type& v) { a.role = v; }
};
} // namespace detail

/// The Links class represents a collection of Link-s. It has a set-like
/// interface that permits insertion, erasure, iteration, find, etc.
/// Additionally, template methods that accept the Left, Right and Role tags
/// facilitate return values that isolate the Left, Right and Role values (as
/// though the container were a container of just that type). Internally, the
/// links are held in a multiindex array to facilitate indexing according to the
/// Id (default), Left, Right and Role values of the links.
template<
  typename id_type,
  typename left_type = id_type,
  typename right_type = left_type,
  typename role_type = int,
  typename base_type = detail::NullLinkBase>
class Links : public detail::LinkContainer<id_type, left_type, right_type, role_type, base_type>
{
  using Parent = detail::LinkContainer<id_type, left_type, right_type, role_type, base_type>;
  template<typename tag>
  using LinkTraits = detail::LinkTraits<id_type, left_type, right_type, role_type, base_type, tag>;

public:
  static const role_type undefinedRole;

  virtual ~Links() = default;

  /// The "Left", "Right" and "Role" tags facilitate access to views into the
  /// container that are sorted according to the left, right or role values,
  /// respectively.
  using Left = detail::Left;
  using Right = detail::Right;
  using Role = detail::Role;

  /// We expose a subset of the base class's types and methods because we use
  /// them for untagged interaction (i.e. methods that do not use a tag) with
  /// the container.
  using iterator = typename Parent::iterator;
  using Link = typename Parent::value_type;
  using LinkBase = base_type;
  using Parent::insert;
  using Parent::size;

  /// Expose template parameters.
  using IdType = id_type;
  using LeftType = left_type;
  using RightType = right_type;
  using RoleType = role_type;

  /// Insertion into the container is performed by passing values for the
  /// base_type object, link id, left value, right value, and role.
  std::pair<iterator, bool> insert(
    const base_type&,
    const id_type&,
    const left_type&,
    const right_type&,
    const role_type& role = undefinedRole);

  /// Since the base type may be large, this method facilitates its insertion
  /// using move semantics.
  std::pair<iterator, bool> insert(
    base_type&&,
    const id_type&,
    const left_type&,
    const right_type&,
    const role_type& role = undefinedRole);

  /// If the base_type is default-constructible, this insertion method allows
  /// you to omit the base_type instance. A new base_type will be used and the
  /// left and right types are passed to the new link using move semantics.
  template<typename return_value = typename std::pair<iterator, bool>>
  typename std::enable_if<std::is_default_constructible<base_type>::value, return_value>::type
  insert(
    const id_type& id,
    const left_type& left,
    const right_type& right,
    const role_type& role = undefinedRole)
  {
    return insert(std::move(base_type()), id, left, right, role);
  }

  /// Check if a link with the input id exists.
  bool contains(const id_type& key) const { return this->find(key) != this->end(); }

  /// Check if a link with the input value matching the tagged search criterion
  /// exists.
  template<typename tag>
  bool contains(const typename LinkTraits<tag>::type& value) const
  {
    auto& self = this->Parent::template get<tag>();
    auto found = self.find(value);
    return found != self.end();
  }

  /// Return the number of links with the input value matching the tagged search
  /// criterion.
  template<typename tag>
  std::size_t size(const typename LinkTraits<tag>::type& value) const
  {
    auto& self = this->Parent::template get<tag>();
    auto range = self.equal_range(value);
    return std::distance(range.first, range.second);
  }

  /// Erase all links matching the input value for the tagged search criterion.
  template<typename tag>
  bool erase_all(const typename LinkTraits<tag>::type& value)
  {
    auto& self = this->Parent::template get<tag>();
    auto to_erase = self.equal_range(value);

    // No elements match |value|, or |self| is empty.
    if (to_erase.first == to_erase.second || to_erase.first == self.end())
    {
      return false;
    }

    self.erase(to_erase.first, to_erase.second);
    return true;
  }

  /// Erase all links matching the input value and role for the tagged search
  /// criterion.
  template<typename tag>
  bool erase_all(const std::tuple<typename LinkTraits<tag>::type, role_type>& value)
  {
    auto& self = this->Parent::template get<tag>();
    auto to_erase = self.equal_range(value);

    // No elements match |value|, or |self| is empty.
    if (to_erase.first == to_erase.second || to_erase.first == self.end())
    {
      return false;
    }

    self.erase(to_erase.first, to_erase.second);
    return true;
  }

  /// Access a link by its id and set its value associated with the tagged
  /// search criterion to a new value.
  template<typename tag>
  bool set(const id_type& id, const typename LinkTraits<tag>::type& value)
  {
    typedef LinkTraits<tag> traits;

    auto linkIt = this->find(id);

    if (linkIt == this->end())
    {
      throw std::out_of_range("Links<id_type, left_type, right_type, role_type, "
                              "base_type>::set(const id_type&, const type& value) : "
                              "no link has this index");
    }

    bool modified = false;

    auto originalValue = traits::value(*linkIt);

    if (originalValue != value)
    {
      struct Modify
      {
        Modify(const typename traits::type& v)
          : value(v)
        {
        }

        void operator()(Link& link) { traits::setValue(link, value); }

        const typename traits::type& value;
      };

      auto& self = this->Parent::template get<tag>();
      auto range = self.equal_range(originalValue);
      for (auto it = range.first; it != range.second; ++it)
      {
        if (it->id == id)
        {
          modified = self.modify(it, Modify(value), Modify(originalValue));
          break;
        }
      }
      assert(modified == true);
    }
    return modified;
  }

  /// Return a set of ids corresponding to the input value for the tagged search
  /// criterion.
  template<typename tag>
  const std::set<std::reference_wrapper<const id_type>> ids(
    const typename LinkTraits<tag>::type& value) const
  {
    std::set<std::reference_wrapper<const id_type>> ids;

    auto& self = this->Parent::template get<tag>();
    auto range = self.equal_range(value);
    for (auto it = range.first; it != range.second; ++it)
    {
      ids.insert(std::cref(it->id));
    }
    return ids;
  }

  /// Access the link with the input id (must be const).
  const Link& at(const id_type& id) const
  {
    auto it = this->find(id);

    if (it == this->end())
    {
      throw std::out_of_range(
        "Links<id_type, left_type, right_type, role_type, base_type>::at(const id_type&) : "
        "no link has this index");
    }

    return *it;
  }

  /// Access a link as its base type (can be non-const).
  LinkBase& value(const id_type& id) { return const_cast<Link&>(this->at(id)); }
  const LinkBase& value(const id_type& id) const { return this->at(id); }

  /// Access a tagged value associated with the input id (must be const; values
  /// can be modified using the "set" method).
  template<typename tag>
  const typename LinkTraits<tag>::type& at(const id_type& id) const
  {
    typedef LinkTraits<tag> traits;

    auto it = this->find(id);

    if (it == this->end())
    {
      throw std::out_of_range(
        "Links<id_type, left_type, right_type, role_type, base_type>::at(const id_type&) : "
        "no link has this index");
    }

    return traits::value(*it);
  }

  /// Given a Left or Right tag and an associated value, return a set of the
  /// other type that links to the input value.
  template<typename tag>
  const std::set<
    std::reference_wrapper<const typename LinkTraits<tag>::other_type>,
    std::less<const typename LinkTraits<tag>::other_type>>
  linked_to(const typename LinkTraits<tag>::type& value) const
  {
    typedef LinkTraits<tag> traits;

    std::set<
      std::reference_wrapper<const typename traits::other_type>,
      std::less<const typename traits::other_type>>
      values;

    auto& self = this->Parent::template get<tag>();
    auto range = self.equal_range(std::make_tuple(value));
    for (auto it = range.first; it != range.second; ++it)
    {
      values.insert(std::cref(LinkTraits<typename traits::OtherTag>::value(*it)));
    }
    return values;
  }

  /// Given a Left or Right tag, an associated value and a role, return a set of
  /// the other type that links to the input value and has the role value.
  template<typename tag>
  const std::set<
    std::reference_wrapper<const typename LinkTraits<tag>::other_type>,
    std::less<const typename LinkTraits<tag>::other_type>>
  linked_to(const typename LinkTraits<tag>::type& value, const role_type& role) const
  {
    typedef LinkTraits<tag> traits;

    std::set<
      std::reference_wrapper<const typename traits::other_type>,
      std::less<const typename traits::other_type>>
      values;

    auto& self = this->Parent::template get<tag>();
    auto range = self.equal_range(std::make_tuple(value, role));
    for (auto it = range.first; it != range.second; ++it)
    {
      values.insert(std::cref(LinkTraits<typename traits::OtherTag>::value(*it)));
    }
    return values;
  }
};

template<
  typename id_type,
  typename left_type,
  typename right_type,
  typename role_type,
  typename base_type>
const role_type Links<id_type, left_type, right_type, role_type, base_type>::undefinedRole =
  std::numeric_limits<role_type>::lowest();

template<
  typename id_type,
  typename left_type,
  typename right_type,
  typename role_type,
  typename base_type>
std::pair<typename Links<id_type, left_type, right_type, role_type, base_type>::iterator, bool>
Links<id_type, left_type, right_type, role_type, base_type>::insert(
  const base_type& base,
  const id_type& id,
  const left_type& left,
  const right_type& right,
  const role_type& role)
{
  return this->insert(Link(base, id, left, right, role));
}

template<
  typename id_type,
  typename left_type,
  typename right_type,
  typename role_type,
  typename base_type>
std::pair<typename Links<id_type, left_type, right_type, role_type, base_type>::iterator, bool>
Links<id_type, left_type, right_type, role_type, base_type>::insert(
  base_type&& base,
  const id_type& id,
  const left_type& left,
  const right_type& right,
  const role_type& role)
{
  return this->insert(Link(std::forward<base_type>(base), id, left, right, role));
}
} // namespace common
} // namespace smtk

#endif // smtk_common_Links_h
