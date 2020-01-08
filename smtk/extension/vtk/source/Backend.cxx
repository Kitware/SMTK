//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/vtk/source/Backend.h"
#include "smtk/extension/vtk/source/Geometry.h"

namespace smtk
{
namespace extension
{
namespace vtk
{
namespace source
{

Backend::format_t Backend::s_empty;

Backend::Backend()
  : m_provider(nullptr)
{
}

Backend::Backend(const std::unique_ptr<smtk::geometry::Geometry>& provider)
{
  try
  {
    const auto& pp = dynamic_cast<const Geometry&>(*provider);
    m_provider = &pp;
  }
  catch (std::bad_cast&)
  {
    m_provider = nullptr;
  }
}

Backend::Backend(const Geometry* provider)
  : m_provider(provider)
{
}

Backend::format_t& Backend::geometry(const smtk::resource::PersistentObject::Ptr& obj)
{
  return m_provider ? m_provider->geometry(obj) : s_empty;
}

Backend::format_t& Backend::geometry(const std::unique_ptr<smtk::geometry::Geometry>& p,
  const smtk::resource::PersistentObject::Ptr& obj)
{
  try
  {
    const auto& provider = dynamic_cast<const Geometry&>(*p);
    format_t& val = provider.geometry(obj);
    return val;
  }
  catch (std::bad_cast& e)
  {
    (void)e; // do nothing
  }
  return s_empty;
}

} // namespace source
} // namespace vtk
} // namespace extension
} // namespace smtk
