//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
/** \file PropertyType.h
 * Typedefs that specify how properties are stored
 * and an enum specifies the storage method.
 */
#ifndef smtk_resource_PropertyType_h
#define smtk_resource_PropertyType_h

#include "smtk/SystemConfig.h"

#include "smtk/common/UUID.h"

#include <map>
#include <string>
#include <vector>

namespace smtk
{
namespace resource
{

/// Primitive storage types for model properties
enum PropertyType
{
  FLOAT_PROPERTY,   //!< Property is an array of floating-point numbers
  STRING_PROPERTY,  //!< Property is an array of strings
  INTEGER_PROPERTY, //!< Property is an array of integers
  INVALID_PROPERTY  //!< Property has no storage.
};

typedef double Float;
typedef std::vector<Float> FloatList;
typedef std::map<std::string, FloatList> FloatData;
typedef std::map<smtk::common::UUID, FloatData> UUIDsToFloatData;
typedef UUIDsToFloatData::iterator UUIDWithFloatProperties;
typedef FloatData::iterator PropertyNameWithFloats;
typedef FloatData::const_iterator PropertyNameWithConstFloats;

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

typedef long Integer;
typedef std::vector<long> IntegerList;
typedef std::map<std::string, IntegerList> IntegerData;
typedef std::map<smtk::common::UUID, IntegerData> UUIDsToIntegerData;
typedef UUIDsToIntegerData::iterator UUIDWithIntegerProperties;
typedef IntegerData::iterator PropertyNameWithIntegers;
typedef IntegerData::const_iterator PropertyNameWithConstIntegers;
} // namespace resource
} // namespace smtk

#endif
