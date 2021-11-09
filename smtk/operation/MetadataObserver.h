//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef smtk_operation_MetadataObserver_h
#define smtk_operation_MetadataObserver_h

#include "smtk/CoreExports.h"

#include "smtk/common/Observers.h"

namespace smtk
{
namespace operation
{
class Metadata;

typedef std::function<void(const Metadata&, bool)> MetadataObserver;

typedef smtk::common::Observers<MetadataObserver> MetadataObservers;
} // namespace operation
} // namespace smtk

#ifndef smtkCore_EXPORTS
extern
#endif
  template class SMTKCORE_EXPORT std::function<void(const smtk::operation::Metadata&)>;

#endif // smtk_operation_MetadataObserver_h
