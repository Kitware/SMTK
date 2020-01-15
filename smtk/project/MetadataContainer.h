//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_project_MetadataContainer_h
#define smtk_project_MetadataContainer_h

#include "smtk/project/Metadata.h"
#include "smtk/project/Tags.h"

namespace smtk
{
namespace project
{

using namespace boost::multi_index;

/// A multi-index container for accessing project metadata. This class is
/// primarily intended to be used in the implementation of
/// smtk::project::Manager only.
typedef boost::multi_index_container<
  Metadata,
  indexed_by<
    ordered_unique<tag<NameTag>, const_mem_fun<Metadata, const std::string&, &Metadata::typeName>>,
    ordered_unique<
      tag<IndexTag>,
      const_mem_fun<Metadata, const smtk::project::Project::Index&, &Metadata::index>>>>
  MetadataContainer;
} // namespace project
} // namespace smtk

#endif
