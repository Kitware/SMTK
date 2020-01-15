//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_project_Container_h
#define smtk_project_Container_h

#include "smtk/project/Project.h"
#include "smtk/project/Tags.h"

#include <boost/multi_index/global_fun.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index_container.hpp>

namespace smtk
{
namespace project
{

/// Global access methods used to sort Container data.
namespace detail
{
inline const smtk::common::UUID& id(const ProjectPtr& r)
{
  return r->id();
}
inline smtk::project::Project::Index index(const ProjectPtr& r)
{
  return r->index();
}
inline std::string name(const smtk::project::ProjectPtr& r)
{
  return r->typeName();
}
inline const std::string& location(const ProjectPtr& r)
{
  return r->location();
}
} // namespace detail

using namespace boost::multi_index;

/// A multi-index container for accessing projects. This class is primarily
/// intended to be used in the implementation of smtk::project::Manager only.
typedef boost::multi_index_container<
  ProjectPtr,
  indexed_by<
    ordered_unique<
      tag<IdTag>,
      global_fun<const ProjectPtr&, const smtk::common::UUID&, &smtk::project::detail::id>>,
    ordered_non_unique<
      tag<IndexTag>,
      global_fun<const ProjectPtr&, smtk::project::Project::Index, &smtk::project::detail::index>>,
    ordered_non_unique<
      tag<NameTag>,
      global_fun<const ProjectPtr&, std::string, &smtk::project::detail::name>>,
    ordered_non_unique<
      tag<LocationTag>,
      global_fun<const ProjectPtr&, const std::string&, &smtk::project::detail::location>>>>
  Container;
} // namespace project
} // namespace smtk

#endif
