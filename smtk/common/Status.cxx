//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/common/Status.h"

namespace smtk
{
namespace common
{

bool Status::markModified()
{
  bool didChange = !m_modified;
  m_modified = true;
  return didChange;
}

bool Status::markFailed()
{
  bool didChange = m_success;
  m_success = false;
  return didChange;
}

Status& Status::operator&=(const Status& other)
{
  m_success &= other.m_success;
  m_modified |= other.m_modified;
  return *this;
}

Status::operator bool() const
{
  return m_success;
}

} // namespace common
} // namespace smtk
