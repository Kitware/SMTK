//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/string/Manager.h"
#include "smtk/string/Token.h"

#include <algorithm>
#include <array>
#include <iostream>

namespace smtk
{
namespace string
{

std::shared_ptr<Manager> Manager::create()
{
  auto manager = std::make_shared<Manager>();
  return manager;
}

Hash Manager::manage(const std::string& s)
{
  std::pair<Hash, bool> hp;
  {
    std::lock_guard<std::mutex> lock(m_writeLock);
    hp = this->computeInternalAndInsert(s);
  }
  if (hp.second)
  {
    m_observers(Event::Managed, hp.first, s, Invalid);
  }
  return hp.first;
}

std::size_t Manager::unmanage(Hash h)
{
  std::size_t num = 0;
  m_writeLock.lock();
  auto it = m_data.find(h);
  if (it == m_data.end())
  {
    m_writeLock.unlock();
    return num;
  }
  auto members = m_sets.find(h);
  if (members != m_sets.end())
  {
    // Erase all sets contained in this set recursively.
    for (auto member : members->second)
    {
      m_writeLock.unlock();
      m_observers(Event::Removed, member, this->value(member), h);
      num += this->unmanage(member);
      m_writeLock.lock();
    }
  }
  m_observers(Event::Unmanaged, h, it->second, Invalid);
  num += m_data.erase(h);
  m_writeLock.unlock();
  return num;
}

bool Manager::hasValue(Hash h) const
{
  auto it = m_data.find(h);
  return (it != m_data.end());
}

const std::string& Manager::value(Hash h) const
{
  static const std::string empty;
  auto it = m_data.find(h);
  if (it == m_data.end())
  {
    return empty;
  }
  return it->second;
}

Hash Manager::find(const std::string& s) const
{
  std::pair<Hash, bool> h;
  {
    std::lock_guard<std::mutex> lock(m_writeLock);
    h = this->computeInternal(s);
  }
  return h.second ? h.first : Invalid;
}

Hash Manager::compute(const std::string& s) const
{
  // std::lock_guard<std::mutex> lock(m_writeLock);
  return this->computeInternal(s).first;
}

Hash Manager::insert(const std::string& set, Hash h)
{
  bool didInsert = false;
  // Verify \a h is managed.
  if (m_data.find(h) == m_data.end())
  {
    return Invalid;
  }
  // Insert \a h into \a set.
  std::pair<Hash, bool> setHash{ Invalid, false };
  {
    std::lock_guard<std::mutex> lock(m_writeLock);
    setHash = this->computeInternalAndInsert(set);
    didInsert = m_sets[setHash.first].insert(h).second;
    if (didInsert)
    {
      m_observers(Event::Inserted, h, this->value(h), setHash.first);
    }
  }
  return setHash.first;
}

bool Manager::insert(Hash set, Hash h)
{
  bool didInsert = false;
  // Verify \a set and \a h are managed.
  if (m_data.find(h) == m_data.end() || m_data.find(set) == m_data.end())
  {
    return didInsert;
  }
  {
    std::lock_guard<std::mutex> lock(m_writeLock);
    didInsert = m_sets[set].insert(h).second;
    if (didInsert)
    {
      m_observers(Event::Inserted, h, this->value(h), set);
    }
  }
  return didInsert;
}

bool Manager::remove(const std::string& set, Hash h)
{
  bool didRemove = false;
  // Verify \a h is managed.
  if (m_data.find(h) == m_data.end())
  {
    return Invalid;
  }
  // Remove \a h from \a set.
  {
    std::lock_guard<std::mutex> lock(m_writeLock);
    auto setHash = this->computeInternalAndInsert(set);
    auto it = m_sets.find(setHash.first);
    // Verify \a setHash is managed.
    if (it == m_sets.end())
    {
      return didRemove;
    }
    didRemove = m_sets[setHash.first].erase(h) > 0;
    if (didRemove)
    {
      m_observers(Event::Removed, h, this->value(h), setHash.first);
      if (m_sets[setHash.first].empty())
      {
        m_sets.erase(setHash.first);
      }
    }
  }
  return didRemove;
}

bool Manager::remove(Hash set, Hash h)
{
  bool didRemove = false;
  std::lock_guard<std::mutex> lock(m_writeLock);
  auto hit = m_data.find(h);
  auto sit = m_sets.find(set);
  // Verify \a h is managed and \a set is a set.
  if (hit == m_data.end() || sit == m_sets.end())
  {
    return false;
  }
  // Remove \a h from \a set.
  didRemove = m_sets[set].erase(h) > 0;
  if (didRemove)
  {
    m_observers(Event::Removed, h, hit->second, set);
    if (m_sets[set].empty())
    {
      m_sets.erase(set);
    }
  }
  return didRemove;
}

bool Manager::contains(const std::string& set, Hash h) const
{
  std::lock_guard<std::mutex> lock(m_writeLock);
  auto setHash = this->computeInternal(set);
  auto sit = m_sets.find(setHash.first);
  return (sit != m_sets.end() && sit->second.find(h) != sit->second.end());
}

bool Manager::contains(Hash set, Hash h) const
{
  if (set == Invalid)
  {
    auto mit = m_data.find(h);
    return mit != m_data.end();
  }
  auto sit = m_sets.find(set);
  return (sit != m_sets.end() && sit->second.find(h) != sit->second.end());
}

bool Manager::verify(Hash& verified, Hash input) const
{
  auto mit = m_data.find(input);
  if (mit != m_data.end())
  {
    verified = input;
    return true;
  }
  auto te = m_translation.find(input);
  if (te != m_translation.end())
  {
    verified = te->second;
    return true;
  }
  verified = Invalid;
  return false;
}

smtk::common::Visit Manager::visitMembers(Visitor visitor, Hash set)
{
  if (!visitor)
  {
    return smtk::common::Visit::Halt;
  }

  m_writeLock.lock();
  if (set == Invalid)
  {
    // Iterate over m_data.
    for (const auto& entry : m_data)
    {
      m_writeLock.unlock();
      if (visitor(entry.first) == smtk::common::Visit::Halt)
      {
        return smtk::common::Visit::Halt;
      }
      m_writeLock.lock();
    }
    m_writeLock.unlock();
    return smtk::common::Visit::Continue;
  }

  // Iterate over m_sets[set].
  auto sit = m_sets.find(set);
  if (sit == m_sets.end())
  {
    m_writeLock.unlock();
    return smtk::common::Visit::Continue;
  }
  for (const auto& entry : sit->second)
  {
    m_writeLock.unlock();
    if (visitor(entry) == smtk::common::Visit::Halt)
    {
      return smtk::common::Visit::Halt;
    }
    m_writeLock.lock();
  }

  m_writeLock.unlock();
  return smtk::common::Visit::Continue;
}

smtk::common::Visit Manager::visitSets(Visitor visitor)
{
  if (!visitor)
  {
    return smtk::common::Visit::Halt;
  }

  m_writeLock.lock();
  // Iterate over m_sets.
  for (const auto& entry : m_sets)
  {
    m_writeLock.unlock();
    if (visitor(entry.first) == smtk::common::Visit::Halt)
    {
      return smtk::common::Visit::Halt;
    }
    m_writeLock.lock();
  }

  m_writeLock.unlock();
  return smtk::common::Visit::Continue;
}

void Manager::setData(
  const std::unordered_map<Hash, std::string>& members,
  const std::unordered_map<Hash, std::unordered_set<Hash>>& sets)
{
  // Remove existing entries.
  // TODO: Notification could be more efficient by only removing entries
  // not identical both before and after.
  std::unordered_map<Hash, std::string>::iterator next;
  m_writeLock.lock();
  auto it = m_data.begin();
  m_writeLock.unlock();
  for (; it != m_data.end(); it = next)
  {
    next = it;
    m_writeLock.lock();
    ++next;
    m_writeLock.unlock();
    this->unmanage(it->first);
  }

  m_writeLock.lock();
  m_data = members;
  m_sets = sets;
  m_writeLock.unlock();

  // Notify observers of new members
  for (const auto& member : m_data)
  {
    m_observers(Event::Managed, member.first, member.second, Invalid);
  }
  // Notify observers of new sets
  for (const auto& set : m_sets)
  {
    for (const auto& child : set.second)
    {
      m_observers(Event::Inserted, child, this->value(child), set.first);
    }
  }
}

void Manager::reset()
{
  m_writeLock.lock();
  m_data.clear();
  m_sets.clear();
  m_writeLock.unlock();
}

std::pair<Hash, bool> Manager::computeInternal(const std::string& s) const
{
  std::pair<Hash, bool> result{ smtk::string::Token::stringHash(s.data(), s.size()), false };
  while (true)
  {
    auto it = m_data.find(result.first);
    if (it == m_data.end())
    {
      return result;
    }
    else if (it->second == s)
    {
      result.second = true;
      return result;
    }
    ++result.first;
  }
  return result;
}

std::pair<Hash, bool> Manager::computeInternalAndInsert(const std::string& s)
{
  std::pair<Hash, bool> result = this->computeInternal(s);
  if (result.first != Invalid)
  {
    m_data[result.first] = s;
    result.second = true;
  }
  return result;
}

std::string eventName(const Manager::Event& e)
{
  static std::array<std::string, 5> names{ { "managed", "inserted", "removed", "unmanaged" } };
  return names[static_cast<int>(e)];
}

Manager::Event eventEnum(const std::string& e)
{
  std::string eventName(e);
  std::transform(eventName.begin(), eventName.end(), eventName.begin(), [](unsigned char c) {
    return std::tolower(c);
  });
  if (eventName.substr(0, 7) == "event::")
  {
    eventName = eventName.substr(7);
  }
  if (eventName == "managed")
  {
    return Manager::Event::Managed;
  }
  if (eventName == "inserted")
  {
    return Manager::Event::Inserted;
  }
  if (eventName == "removed")
  {
    return Manager::Event::Removed;
  }
  // else if (eventName == "unmanaged")
  return Manager::Event::Unmanaged;
}

std::ostream& operator<<(std::ostream& os, const Manager::Event& e)
{
  os << eventName(e);
  return os;
}

} // namespace string
} // namespace smtk
