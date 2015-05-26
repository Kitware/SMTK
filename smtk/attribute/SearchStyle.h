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

namespace smtk {
  namespace attribute {

/**\brief How should searches for items be conducted?
  *
  */
enum SearchStyle
{
  NO_CHILDREN,     //!< Search only the attribute, not its children
  ACTIVE_CHILDREN, //!< Search the attribute, descending active children
  ALL_CHILDREN     //!< Search the attribute, descending all children
};

  } // attribute namespace
} // smtk namespace

#endif // __smtk_attribute_SearchStyle_h
