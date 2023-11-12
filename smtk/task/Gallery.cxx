//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/task/Gallery.h"

#include "smtk/io/Logger.h"

namespace smtk
{
namespace task
{
Gallery::Gallery(Manager* manager)
  : m_manager(manager)
{
}

bool Gallery::add(const Worklet::Ptr& worklet, bool makeUnique)
{
  if (!worklet)
  {
    return false;
  }
  // Only add the worklet iff it has a name and is owned by this manager
  if ((worklet->manager().get() != m_manager) || (worklet->name().empty()))
  {
    smtkWarningMacro(
      smtk::io::Logger::instance(),
      "Worklet: " << worklet->name() << " is managed by a different Task Manager or is not named");
    return false;
  }

  std::string wname = (makeUnique ? this->createUniqueName(worklet->name()) : worklet->name());
  if (!makeUnique)
  {
    // See if the worklet's name already exists in the gallery
    auto it = m_worklets.find(wname);
    if (it != m_worklets.end())
    {
      smtkWarningMacro(
        smtk::io::Logger::instance(),
        "A Worklet with name: " << worklet->name()
                                << " is already part of the Task Manager's gallery");
      return false;
    }
  }

  m_worklets[wname] = worklet;
  return true;
}

bool Gallery::remove(const Worklet::Ptr& worklet)
{
  if (!worklet)
  {
    return false;
  }
  auto it = m_worklets.find(worklet->name());
  if (it == m_worklets.end())
  {
    smtkWarningMacro(
      smtk::io::Logger::instance(),
      "Worklet with name: " << worklet->name() << " could not be found in Task Manager's gallery");
    return false; // there is no worklet by that name
  }
  if (it->second != worklet)
  {
    smtkWarningMacro(
      smtk::io::Logger::instance(),
      "Found a different Worklet with name: " << worklet->name() << " in Task Manager's gallery");
    return false;
  }
  m_worklets.erase(it);
  return true;
}

Worklet::Ptr Gallery::find(const std::string& name) const
{
  auto it = m_worklets.find(name);
  if (it == m_worklets.end())
  {
    return Worklet::Ptr();
  }
  return it->second;
}

bool Gallery::rename(const Worklet::Ptr& worklet, const std::string& newName)
{
  if (!worklet)
  {
    return false;
  }

  // First check to make sure the newName doesn't already exists
  auto it = m_worklets.find(newName);
  if (it != m_worklets.end())
  {
    smtkWarningMacro(
      smtk::io::Logger::instance(),
      "A Worklet with requested new name: " << newName
                                            << " is already part of the Task Manager's gallery");
    return false;
  }

  // Next see if the worklet is owned by the Gallery's Task Manager or is not named
  if ((worklet->manager().get() != m_manager) || (worklet->name().empty()))
  {
    smtkWarningMacro(
      smtk::io::Logger::instance(),
      "Worklet: " << worklet->name() << " is managed by a different Task Manager or is not named");
    return false;
  }

  it = m_worklets.find(worklet->name());
  if (it == m_worklets.end())
  {
    smtkWarningMacro(
      smtk::io::Logger::instance(),
      "Worklet with original name: " << worklet->name()
                                     << " was not initially part of the Task Manager's gallery");
  }
  else
  {
    m_worklets.erase(it);
  }
  worklet->setName(newName);
  m_worklets[newName] = worklet;
  return true;
}

std::string Gallery::createUniqueName(const std::string& name) const
{
  // First see if the name is not currently in use
  auto it = m_worklets.find(name);
  if (it == m_worklets.end())
  {
    return name;
  }

  int i = 0;
  std::string base = name, newName;
  base.append(m_uniqueNameSeparator);
  while (it != m_worklets.end())
  {
    std::ostringstream n;
    n << i++;
    newName = base + n.str();
    it = m_worklets.find(newName);
  }
  return newName;
}

} // namespace task
} // namespace smtk
