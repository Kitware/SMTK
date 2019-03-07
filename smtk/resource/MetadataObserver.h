//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_resource_MetadataObserver_h
#define __smtk_resource_MetadataObserver_h

#include "smtk/CoreExports.h"

#include "smtk/common/Observers.h"

namespace smtk
{
namespace resource
{
class Metadata;

typedef std::function<void(const Metadata&, bool)> MetadataObserver;

typedef smtk::common::Observers<MetadataObserver> MetadataObservers;
}
}

#ifndef smtkCore_EXPORTS
extern
#endif
  template class SMTKCORE_EXPORT std::function<void(const smtk::resource::Metadata&)>;

#endif // __smtk_resource_MetadataObserver_h
