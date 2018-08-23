//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_resource_MetadataContainer_h
#define smtk_resource_MetadataContainer_h

#include "smtk/resource/Container.h"
#include "smtk/resource/Metadata.h"

namespace smtk
{
namespace resource
{

using namespace boost::multi_index;

/// A multi-index container for accessing resource metadata. This class is
/// primarily intended to be used in the implementation of
/// smtk::resource::Manager only.
typedef boost::multi_index_container<
  Metadata,
  indexed_by<
    ordered_unique<tag<NameTag>, const_mem_fun<Metadata, const std::string&, &Metadata::typeName> >,
    ordered_unique<tag<IndexTag>,
      const_mem_fun<Metadata, const smtk::resource::Resource::Index&, &Metadata::index> > > >
  MetadataContainer;
}
}

#endif
