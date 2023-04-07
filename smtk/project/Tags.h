//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef smtk_project_Tags_h
#define smtk_project_Tags_h

#include "smtk/CoreExports.h"

namespace smtk
{
namespace project
{

/// Tags used to access Project data from multiindex arrays.
struct SMTKCORE_EXPORT IdTag
{
};
struct SMTKCORE_EXPORT IndexTag
{
};
struct SMTKCORE_EXPORT LocationTag
{
};
struct SMTKCORE_EXPORT NameTag
{
};
struct SMTKCORE_EXPORT RoleTag
{
};
} // namespace project
} // namespace smtk

#endif
