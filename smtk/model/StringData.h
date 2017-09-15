//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_model_StringData_h
#define __smtk_model_StringData_h
/** \file StringData.h
 * Typedefs that specify how string properties are stored.
 */

#include "smtk/SystemConfig.h"

#include "smtk/common/UUID.h"

#include <map>
#include <string>
#include <vector>

namespace smtk
{
namespace model
{

/// Use std::string for holding string values.
typedef std::string String;
/// Use vectors of String objects for holding string properties on model entities.
typedef std::vector<String> StringList;
/// A dictionary of property names mapped to their values (string vectors)
typedef std::map<std::string, StringList> StringData;
/// A dictionary of model entities mapped to all the string properties defined on them.
typedef std::map<smtk::common::UUID, StringData> UUIDsToStringData;
/// A convenient typedef that describes how an iterator to model-entity string properties is used.
typedef UUIDsToStringData::iterator UUIDWithStringProperties;
/// A convenient typedef that describes how the iterator to one string property is used.
typedef StringData::iterator PropertyNameWithStrings;
/// A convenient typedef that describes how the const_iterator to one string property is used.
typedef StringData::const_iterator PropertyNameWithConstStrings;

} // namespace model
} // namespace smtk

#endif // __smtk_model_StringData_h
