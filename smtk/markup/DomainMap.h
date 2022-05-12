//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_markup_DomainMap_h
#define smtk_markup_DomainMap_h

#include "smtk/markup/Exports.h"
#include "smtk/string/Token.h"

#include <memory>
#include <unordered_map>
#include <unordered_set>

namespace smtk
{
namespace markup
{

class Domain;

/**\brief Container for all the namespaces a resource presents for components to share.
  *
  * Components may own their own private domains, but if multiple components are
  * intended to share a domain, the resource should own and manage it via this map.
  */
class SMTKMARKUP_EXPORT DomainMap
{
public:
  DomainMap() = default;
  ~DomainMap() = default;
  DomainMap(const DomainMap&) = default;
  DomainMap& operator=(const DomainMap&) = default;

  /// Return true if the \a space exists in this map.
  bool contains(const smtk::string::Token& space) const;
  /**\brief Insert the \a domain instance into the map, returning
   *        true on success (and failure if \a space already existed).
   */
  bool insert(const smtk::string::Token& space, const std::shared_ptr<Domain>& domain);
  /// Find a domain for the given namespace, returning null if none exists.
  std::shared_ptr<Domain> find(const smtk::string::Token& space) const;
  /// Find a domain for the given namespace, returning null if none exists.
  std::shared_ptr<Domain> operator[](const smtk::string::Token& space) const;

  /// A convenience to downcast a domain to the type expected for \a space.
  template<typename DomainType>
  std::shared_ptr<DomainType> findAs(const smtk::string::Token& space) const
  {
    auto domain = this->find(space);
    return std::dynamic_pointer_cast<DomainType>(domain);
  }

  /// Create a domain of the given \a DomainType and insert it into the map at \a space.
  template<typename DomainType, typename... Args>
  bool create(const smtk::string::Token& space, Args&&... args)
  {
    auto domain = std::make_shared<DomainType>(std::forward<Args>(args)...);
    return this->insert(space, domain);
  }

  /// Return a set of all the domain "names" in the map.
  std::unordered_set<smtk::string::Token> keys() const;

protected:
  std::unordered_map<smtk::string::Token, std::shared_ptr<Domain>> m_domains;
};

} // namespace markup
} // namespace smtk

#endif // smtk_markup_DomainMap_h
