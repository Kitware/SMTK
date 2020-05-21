//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtk/session/opencascade/Session.h"

#include "TCollection_ExtendedString.hxx"

namespace smtk
{
namespace session
{
namespace opencascade
{

Session::Session()
  : m_document(new TDocStd_Document(TCollection_ExtendedString(/* document format */ "smtk")))
  , m_shapeCounters{ 0 }
{
}

Session::~Session()
{
}

const TopoDS_Shape* Session::findShape(const smtk::common::UUID& uid) const
{
  auto it = m_storage.find(uid);
  if (it != m_storage.end())
  {
    return &it->second;
  }
  return nullptr;
}

TopoDS_Shape* Session::findShape(const smtk::common::UUID& uid)
{
  auto it = m_storage.find(uid);
  if (it != m_storage.end())
  {
    return &it->second;
  }
  return nullptr;
}

smtk::common::UUID Session::findID(const TopoDS_Shape& shape) const
{
  auto it = m_reverse.find(shape.HashCode(std::numeric_limits<Standard_Integer>::max()));
  if (it != m_reverse.end())
  {
    return it->second;
  }
  return smtk::common::UUID::null();
}

void Session::addShape(const smtk::common::UUID& uid, TopoDS_Shape& storage)
{
  if (uid.isNull() || storage.IsNull())
  {
    return;
  }

  m_storage[uid] = storage;
  m_reverse[storage.HashCode(std::numeric_limits<Standard_Integer>::max())] = uid;
}

bool Session::removeShape(const smtk::common::UUID& uid)
{
  if (uid.isNull())
  {
    return false;
  }

  const auto it = m_storage.find(uid);
  if (it == m_storage.end())
  {
    return false;
  }

  m_reverse.erase(it->second.HashCode(std::numeric_limits<Standard_Integer>::max()));
  m_storage.erase(it);
  return true;
}

} // namespace opencascade
} // namespace session
} // namespace smtk
