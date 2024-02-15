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

bool CopyOptions::setCopyComponents(bool copy)
{
  if (copy == m_copyComponents)
  {
    return false;
  }
  m_copyComponents = copy;
  return true;
}

bool CopyOptions::setCopyProperties(bool copy)
{
  if (copy == m_copyProperties)
  {
    return false;
  }
  m_copyProperties = copy;
  return true;
}

bool CopyOptions::setCopyTemplateData(bool copy)
{
  if (copy == m_copyTemplateData)
  {
    return false;
  }
  m_copyTemplateData = copy;
  return true;
}

bool CopyOptions::setCopyTemplateVersion(bool copy)
{
  if (copy == m_copyTemplateVersion)
  {
    return false;
  }
  m_copyTemplateVersion = copy;
  return true;
}

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

} // namespace resource
} // namespace smtk
