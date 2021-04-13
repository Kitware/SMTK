//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_attribute_SearchStyle_h
#define __smtk_attribute_SearchStyle_h

#include "smtk/CoreExports.h"

namespace smtk
{
namespace attribute
{

/// \brief How should searches for items be conducted?
///
/// Used by find methods in Attribute and Item derived classes

enum SearchStyle
{
  IMMEDIATE = 0,   ///< Search only the top level items of an item or attribute
  NO_CHILDREN = 0, ///< Search only the top level items of an item or attribute (Deprecated!)
  RECURSIVE = 1,   ///< Recursively search for the item regardless if it is active or not
  ALL_CHILDREN =
    1, ///< Recursively search for the item regardless if it is active or not (Deprecated!)
  IMMEDIATE_ACTIVE = 2, ///< Search only the top level active items of an item or attribute
  RECURSIVE_ACTIVE = 3, ///< Recursively search for an active item
  ACTIVE_CHILDREN = 3   ///< Recursively search for an active item (Depricated!)
};

} // namespace attribute
} // namespace smtk

#endif // __smtk_attribute_SearchStyle_h
