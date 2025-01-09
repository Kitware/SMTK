//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_attribute_ReferenceItemIteratorTemplate_h
#define smtk_attribute_ReferenceItemIteratorTemplate_h

#include "smtk/attribute/ReferenceItem.h"

namespace smtk
{
namespace attribute
{

/**\brief Provides standard const_iterator interface for ReferenceItem-based attribute item types.
 *
 * This template class implements the const_iterator API expected by the STL for a container, which
 * is some type based on ReferenceItem. It is implemented as a wrapper around ReferenceItem's
 * existing const_iterator implementation and add the ability to return the expected value type of
 * the item's underlying storage.
 */
template<typename H>
class ReferenceItemConstIteratorTemplate
{
  friend class ComponentItem;
  friend class ResourceItem;

public:
  typedef std::random_access_iterator_tag iterator_category;
  typedef const typename H::Ptr value_type;
  typedef value_type reference;
  typedef value_type pointer;
  typedef std::ptrdiff_t difference_type;

  ReferenceItemConstIteratorTemplate() = default;
  ReferenceItemConstIteratorTemplate(const ReferenceItemConstIteratorTemplate<H>& it) = default;

  ~ReferenceItemConstIteratorTemplate() = default;

  ReferenceItemConstIteratorTemplate<H>& operator=(const ReferenceItemConstIteratorTemplate<H>& it)
  {
    this->m_refItemIterator = it.m_refItemIterator;
    return *this;
  }

  ReferenceItemConstIteratorTemplate<H>& operator++()
  {
    this->m_refItemIterator++;
    return *this;
  }

  ReferenceItemConstIteratorTemplate<H>& operator--()
  {
    this->m_refItemIterator--;
    return *this;
  }

  ReferenceItemConstIteratorTemplate<H> operator++(int)
  {
    this->operator++();
    return *this;
  }

  ReferenceItemConstIteratorTemplate<H> operator--(int)
  {
    this->operator--();
    return *this;
  }

  ReferenceItemConstIteratorTemplate<H> operator+(const difference_type& d) const
  {
    ReferenceItemConstIteratorTemplate<H> result;
    result.m_refItemIterator = this->m_refItemIterator + d;
    return result;
  }
  ReferenceItemConstIteratorTemplate<H> operator-(const difference_type& d) const
  {
    ReferenceItemConstIteratorTemplate<H> result;
    result.m_refItemIterator = this->m_refItemIterator - d;
    return result;
  }

  reference operator*() const
  {
    return this->m_refItemIterator.template as<typename value_type::element_type>();
  }

  pointer operator->() const
  {
    return this->m_refItemIterator.template as<typename value_type::element_type>();
  }

  reference operator[](const difference_type&)
  {
    return this->m_refItemIterator.template as<typename value_type::element_type>();
  }

  /// Return the iterator's target object cast to a shared-pointer type \a K.
  template<typename K>
  std::shared_ptr<K> as() const
  {
    // Type-cast the result of the "*" operator above:
    return std::dynamic_pointer_cast<K>(this->ReferenceItemConstIteratorTemplate<H>::operator*());
  }

  bool isSet() const { return this->m_refItemIterator.isSet(); }

  difference_type operator-(const ReferenceItemConstIteratorTemplate<H>& right)
  {
    return this->m_refItemIterator - right.m_refItemIterator;
  }

  bool operator<(const ReferenceItemConstIteratorTemplate<H>& right)
  {
    return this->m_refItemIterator < right.m_refItemIterator;
  }

  bool operator>(const ReferenceItemConstIteratorTemplate<H>& right)
  {
    return this->m_refItemIterator > right.m_refItemIterator;
  }

  bool operator<=(const ReferenceItemConstIteratorTemplate<H>& right)
  {
    return this->m_refItemIterator <= right.m_refItemIterator;
  }

  bool operator>=(const ReferenceItemConstIteratorTemplate<H>& right)
  {
    return this->m_refItemIterator >= right.m_refItemIterator;
  }

  bool operator==(const ReferenceItemConstIteratorTemplate<H>& right)
  {
    return this->m_refItemIterator == right.m_refItemIterator;
  }

  bool operator!=(const ReferenceItemConstIteratorTemplate<H>& right)
  {
    return this->m_refItemIterator != right.m_refItemIterator;
  }

private:
  ReferenceItem::const_iterator m_refItemIterator;
};

} // namespace attribute
} // namespace smtk

#endif // smtk_attribute_ReferenceItemIteratorTemplate_h
