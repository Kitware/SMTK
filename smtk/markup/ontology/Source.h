//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_markup_ontology_Source_h
#define smtk_markup_ontology_Source_h

#include "smtk/SystemConfig.h" // to silence MSVC warning C4521
#include "smtk/markup/Exports.h"

#include <set>
#include <string>
#include <vector>

namespace smtk
{
namespace markup
{
/// Utility classes that import ontology classes for widget autocompletion.
namespace ontology
{

/// An ontology class identifier.
struct SMTKMARKUP_EXPORT Identifier
{
  /// The human-readable name of the class.
  std::string name;
  /// The IRI of the class.
  std::string url;
  /// The base of the class (or a base if multiple inheritance).
  std::string base;
  /// A human-readable description of the class, in markdown format.
  std::string description;
  /// All the bases of the class (if multiple inheritance).
  mutable std::set<std::string> collection;

  /// A comparator for sorting identifiers by name.
  bool operator<(const Identifier& other) const;
};

/// An ontology class-relationship identifier.
struct SMTKMARKUP_EXPORT Relation
{
  /// The IRI of the relationship.
  std::string url;
  /// The human-readable name of the relationship.
  std::string name;
  /// A description of the relationship's domain.
  std::string domainUrl;
  /// A description of the relationship's range.
  std::string rangeUrl;
  /// Whether the nature of the relationship allows plurality.
  std::size_t plural; // 0 = unlimited, 1 = singular, 2+ = fixed maximum

  /// A comparator for sorting relationships by name.
  bool operator<(const Relation& other) const;
};

/// A source of ontology models.
/// Instances are created by plugins and passed to registerOntology().
class SMTKMARKUP_EXPORT Source
{
public:
  Source(
    const std::string& url,
    const std::string& name,
    std::vector<Identifier>&& classes,
    std::vector<Relation>&& relations)
    : m_url(url)
    , m_name(name)
    , m_classes(classes)
    , m_relations(relations)
  {
  }
  Source(const std::string& url, const std::string& name)
    : m_url(url)
    , m_name(name)
  {
  }
  Source() = default;
  Source(const Source&) = default;
  virtual ~Source() = default;

  const std::string& url() const { return m_url; }
  const std::string& name() const { return m_name; }
  const std::vector<Identifier>& classes() const { return m_classes; }
  const std::vector<Relation>& relations() const { return m_relations; }

  static const Source& findByName(const std::string& name);
  static const Source& findByURL(const std::string& url);
  static bool registerSource(const Source& src);

protected:
  std::string m_url;
  std::string m_name;
  std::vector<Identifier> m_classes;
  std::vector<Relation> m_relations;
};

} // namespace ontology
} // namespace markup
} // namespace smtk

#endif // smtk_markup_ontology_Source_h
