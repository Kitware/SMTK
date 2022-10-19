//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_common_update_Factory_h
#define smtk_common_update_Factory_h

#include "smtk/string/Token.h"

#include <cstddef> // for std::size_t
#include <set>
#include <unordered_map>

namespace smtk
{
namespace common
{
namespace update
{

/**\brief Common class to hold a dictionary of update methods
  *       for various elements in SMTK.
  *
  * The template parameter is the signature of a function to which
  * a target element and output object are passed.
  *
  * The template parameter must name a type with a default constructor
  * that produces an object which can be implicitly converted to a
  * false-like value.
  * For example, an item updater might use an UpdaterSignature
  * of std::function<bool(const Item*, Item*)>; the default constructor
  * produces an empty functor that is implicitly converted to nullptr.
  * This property is used to produce an invalid factory Entry when
  * no updater exists.
  */
template<typename UpdaterSignature>
class Factory
{
public:
  /**\brief An entry in the factory holding an update method and
    *       all the metadata required to index the things the method
    *       applies to.
    */
  struct Entry
  {
    Entry() = default;
    Entry(
      std::size_t appliesToVersionMin,
      std::size_t appliesToVersionMax,
      std::size_t producesVersion,
      UpdaterSignature update)
      : AppliesToVersionMin(appliesToVersionMin)
      , AppliesToVersionMax(appliesToVersionMax)
      , ProducesVersion(producesVersion)
      , Update(update)
    {
    }
    /// The lowest revision of the input element this updater applies to.
    std::size_t AppliesToVersionMin = 0;
    /// The highest revision of the input element this updater applies to.
    std::size_t AppliesToVersionMax = 0;
    /// The version of an element produced by invoking the updater on a target.
    std::size_t ProducesVersion = 0;
    /// The method to invoke on a target to update it.
    UpdaterSignature Update;

    /// True when an entry has a valid updater and revision range.
    bool isValid() const;

    /// An entry is truthy when it is valid.
    operator bool() const { return this->isValid(); }
    /// A comparator for insertion into ordered containers.
    bool operator<(const Entry& other) const;
  };

  /// Return an entry with the appropriate updater (or an invalid entry).
  const Entry& find(
    smtk::string::Token elementSpec,
    std::size_t resultElementVersion,
    std::size_t inputElementVersion);

  /// Given a target, return the set of versions which updaters can produce as output.
  std::set<std::size_t> canProduce(smtk::string::Token elementSpec) const;

  /// Given a target, return the set of versions which updaters can accept as input.
  std::set<std::size_t> canAccept(smtk::string::Token elementSpec) const;

  bool registerUpdater(
    smtk::string::Token target,
    std::size_t appliesToVersionMin,
    std::size_t appliesToVersionMax,
    std::size_t producesVersion,
    UpdaterSignature updater);

protected:
  std::unordered_map<smtk::string::Token, std::set<Entry>> m_entries;
};

template<typename UpdaterSignature>
bool Factory<UpdaterSignature>::Entry::isValid() const
{
  bool valid = this->Update && this->AppliesToVersionMin <= this->AppliesToVersionMax &&
    this->ProducesVersion > this->AppliesToVersionMax;
  return valid;
}

template<typename UpdaterSignature>
bool Factory<UpdaterSignature>::Entry::operator<(const Entry& other) const
{
  bool isLessThan =
    (this->ProducesVersion < other.ProducesVersion ||
     (this->ProducesVersion == other.ProducesVersion &&
      (this->AppliesToVersionMin < other.AppliesToVersionMin ||
       (this->AppliesToVersionMin == other.AppliesToVersionMin &&
        (this->AppliesToVersionMax < other.AppliesToVersionMax ||
         (this->AppliesToVersionMax == other.AppliesToVersionMax &&
          &this->Update < &other.Update))))));
  return isLessThan;
}

template<typename UpdaterSignature>
const typename Factory<UpdaterSignature>::Entry& Factory<UpdaterSignature>::find(
  smtk::string::Token elementSpec,
  std::size_t resultElementVersion,
  std::size_t inputElementVersion)
{
  static thread_local Factory<UpdaterSignature>::Entry invalid;
  auto eit = m_entries.find(elementSpec);
  if (eit == m_entries.end())
  {
    return invalid;
  }
  // For now, just return the first match that meets requirements.
  // In the future, we may get fancier.
  for (const auto& updater : eit->second)
  {
    if (!updater.isValid())
    {
      continue;
    }
    if (
      updater.AppliesToVersionMin <= inputElementVersion &&
      updater.AppliesToVersionMax >= inputElementVersion &&
      updater.ProducesVersion == resultElementVersion)
    {
      return updater;
    }
  }
  return invalid;
}

/// Given a target, return the set of versions which updaters can produce as output.
template<typename UpdaterSignature>
std::set<std::size_t> Factory<UpdaterSignature>::canProduce(smtk::string::Token elementSpec) const
{
  std::set<std::size_t> versions;
  auto eit = m_entries.find(elementSpec);
  if (eit == m_entries.end())
  {
    return versions;
  }
  for (const auto& updater : eit->second)
  {
    if (!updater.isValid())
    {
      continue;
    }
    versions.insert(updater.ProducesVersion);
  }
  return versions;
}

/// Given a target, return the set of versions which updaters can accept as input.
template<typename UpdaterSignature>
std::set<std::size_t> Factory<UpdaterSignature>::canAccept(smtk::string::Token elementSpec) const
{
  std::set<std::size_t> versions;
  auto eit = m_entries.find(elementSpec);
  if (eit == m_entries.end())
  {
    return versions;
  }
  for (const auto& updater : eit->second)
  {
    if (!updater.isValid())
    {
      continue;
    }
    for (std::size_t vv = updater.AppliesToVersionMin; vv <= updater.AppliesToVersionMax; ++vv)
    {
      versions.insert(vv);
    }
  }
  return versions;
}

template<typename UpdaterSignature>
bool Factory<UpdaterSignature>::registerUpdater(
  smtk::string::Token target,
  std::size_t appliesToVersionMin,
  std::size_t appliesToVersionMax,
  std::size_t producesVersion,
  UpdaterSignature update)
{
  // Refuse to insert ill-formed metadata.
  if (
    !update || target.data().empty() || appliesToVersionMin > appliesToVersionMax ||
    appliesToVersionMax >= producesVersion)
  {
    return false;
  }
  Entry ee(appliesToVersionMin, appliesToVersionMax, producesVersion, update);
  m_entries[target].insert(ee);
  return true;
}

} // namespace update
} // namespace common
} // namespace smtk

#endif // smtk_common_update_Factory_h
