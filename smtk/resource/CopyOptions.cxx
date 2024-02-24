//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/resource/CopyOptions.h"

#include "smtk/resource/Resource.h"

#include "smtk/io/Logger.h"

namespace smtk
{
namespace resource
{

CopyOptions::CopyOptions()
  : m_log(new smtk::io::Logger)
  , m_deleteLog(true)
{
}

CopyOptions::CopyOptions(smtk::io::Logger& log)
  : m_log(&log)
  , m_deleteLog(false)
{
}

CopyOptions::~CopyOptions()
{
  if (m_deleteLog)
  {
    delete m_log;
  }
  m_log = nullptr;
}

#define smtkCopySetMacro(Class, Thing)                                                             \
  bool Class ::setCopy##Thing(bool copy)                                                           \
  {                                                                                                \
    if (copy == m_copy##Thing)                                                                     \
    {                                                                                              \
      return false;                                                                                \
    }                                                                                              \
    m_copy##Thing = copy;                                                                          \
    return true;                                                                                   \
  }

smtkCopySetMacro(CopyOptions, Location);
smtkCopySetMacro(CopyOptions, Components);
smtkCopySetMacro(CopyOptions, Properties);
smtkCopySetMacro(CopyOptions, Geometry);
smtkCopySetMacro(CopyOptions, TemplateData);
smtkCopySetMacro(CopyOptions, TemplateVersion);
smtkCopySetMacro(CopyOptions, Links);

bool CopyOptions::setCopyUnitSystem(CopyType copy)
{
  if (copy == m_copyUnitSystem)
  {
    return false;
  }
  m_copyUnitSystem = copy;
  return true;
}

bool CopyOptions::clearLinkRolesToExclude()
{
  if (m_linkRolesToExclude.empty())
  {
    return false;
  }
  m_linkRolesToExclude.clear();
  return true;
}

bool CopyOptions::addLinkRoleToExclude(smtk::resource::Links::RoleType linkRole)
{
  bool didInsert = m_linkRolesToExclude.insert(linkRole).second;
  return didInsert;
}

bool CopyOptions::removeLinkRoleToExclude(smtk::resource::Links::RoleType linkRole)
{
  bool didRemove = m_linkRolesToExclude.erase(linkRole) > 0;
  return didRemove;
}

bool CopyOptions::shouldOmitId(const smtk::common::UUID& sourceId) const
{
  return m_omit.find(sourceId) != m_omit.end();
}

void CopyOptions::omitComponents(const std::shared_ptr<const Resource>& resource)
{
  // Do not attempt to re-omit what is already omitted.
  if (
    !resource || m_omitComponentsResources.find(resource->id()) != m_omitComponentsResources.end())
  {
    return;
  }

  Component::Visitor omitComponents = [this](const Component::Ptr& comp) {
    m_omit.insert(comp->id());
  };
  resource->visit(omitComponents);
  m_omitComponentsResources.insert(resource->id());
}

} // namespace resource
} // namespace smtk
