//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_project_MetadataObserver_h
#define __smtk_project_MetadataObserver_h

#include "smtk/CoreExports.h"

#include "smtk/common/Observers.h"

namespace smtk
{
namespace project
{
class Metadata;

/// An observer for the addition and removal of Project Metadata from a Project
/// Manager. The boolean input is true when Metadata is added and false when
/// removed.
typedef std::function<void(const Metadata&, bool)> MetadataObserver;

typedef smtk::common::Observers<MetadataObserver> MetadataObservers;
} // namespace project
} // namespace smtk

#ifndef smtkCore_EXPORTS
extern
#endif
  template class SMTKCORE_EXPORT std::function<void(const smtk::project::Metadata&)>;

#endif
