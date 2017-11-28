//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_operation_MetadataContainer_h
#define smtk_operation_MetadataContainer_h

#include "smtk/operation/Metadata.h"
#include "smtk/operation/NewOp.h"

#include <boost/multi_index/global_fun.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index_container.hpp>

namespace smtk
{
namespace operation
{

/// Tags used to access Container data.
struct IndexTag
{
};
struct NameTag
{
};

using namespace boost::multi_index;

/// A multi-index container for accessing operator metadata. This class is
/// primarily intended to be used in the implementation of
/// smtk::operation::Manager only.
typedef boost::multi_index_container<
  Metadata,
  indexed_by<ordered_unique<tag<NameTag>,
               const_mem_fun<Metadata, const std::string&, &Metadata::uniqueName> >,
    ordered_unique<tag<IndexTag>,
               const_mem_fun<Metadata, const smtk::operation::NewOp::Index&, &Metadata::index> > > >
  MetadataContainer;
}
}

#endif // smtk_operation_MetadataContainer_h
